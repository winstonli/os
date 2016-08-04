#include "acpi.h"
#include "assert.h"
#include "common/checksum.h"
#include "string.h"
#include "vm/vm.h"

struct PACKED rsdp_descriptor {
  char signature[8];  // "RSD PTR ", no null terminator
  uint8_t checksum;
  char OEMID[6];
  uint8_t revision;     // 0 for acpi 1.0, 2 for acpi 2.0 to 6.1
  uint32_t rsdt_paddr;  // address of root system description table
};

struct PACKED rsdp_descriptor_v2 : rsdp_descriptor {
  uint32_t length;
  uint64_t xsdt_paddr;
  uint8_t ext_checksum;
  uint8_t reserved[3];
};

// 16-bit pointer to ebda start physical addr right-shifted 4 places
// (so shift left 4 to get actual physical addr after deref)
#define EBDA_PTR_PADDR reinterpret_cast<uint16_t *>(0x040e)

static char *get_ebda_addr() {
  uint16_t *ebda_ptr_vaddr = vm::paddr_to_vaddr(EBDA_PTR_PADDR);
  uint16_t ebda_paddr = *ebda_ptr_vaddr << 4;
  return vm::paddr_to_vaddr(reinterpret_cast<char *>(ebda_paddr));
}

static char *search_for_rsdp(char *begin, char *end) {
  // check is 16-byte aligned
  assert(reinterpret_cast<uintptr_t>(begin) % 16 == 0);
  for (; begin < end; begin += 16) {
    if (!memcmp(begin, "RSD PTR ", 8)) {
      klog_debug("Found Root System Description Pointer at %x", begin);
      return begin;
    }
  }
  return nullptr;
}

static rsdp_descriptor *find_rsdp() {
  // places to search (only 16-byte boundaries):
  //  - ebda_addr to ebda_addr + 1024
  //  - 0xE0000 to 0xFFFFF

  char *ebda_addr = get_ebda_addr();
  klog_debug("Extended BIOS data area is located at %x", ebda_addr);

  char *bios_area_start = vm::paddr_to_vaddr(reinterpret_cast<char *>(0xe0000));
  char *bios_area_end = vm::paddr_to_vaddr(reinterpret_cast<char *>(0xfffff));

  auto res = search_for_rsdp(ebda_addr, ebda_addr + 1024);
  if (!res) {
    res = search_for_rsdp(bios_area_start, bios_area_end);
    if (!res) {
      klog_err("No Root System Description Pointer Found!");
    }
  }
  return reinterpret_cast<rsdp_descriptor *>(res);
}

struct PACKED acpi_sdt_header_t {
  char signature[4];  // not null-terminated!
  uint32_t length;    // total size of the table including this header
  uint8_t revision;
  uint8_t checksum;
  char oem_id[6];        // not null-terminated!
  char oem_table_id[8];  // not null-terminated!
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
};

struct rsdt_result_t {
  acpi_sdt_header_t *rsdt_vaddr;
  bool is_xsdt;
};

static rsdt_result_t find_rsdt() {
  // first find the rsdp, then parse it to find a pointer to the rsdt
  auto rsdp_vaddr = find_rsdp();

  // do rsdp v1 checksum
  uint8_t checksum_result = checksum(rsdp_vaddr, sizeof(rsdp_descriptor));
  if (checksum_result != 0) {
    klog_warn("RSDP checksum is invalid");
  }

  uint64_t rsdt_paddr = 0;
  switch (rsdp_vaddr->revision) {
    case 0:
      rsdt_paddr = rsdp_vaddr->rsdt_paddr;
      break;
    case 2: {
      auto rsdp2_ptr = static_cast<rsdp_descriptor_v2 *>(rsdp_vaddr);

      // do rsdp v2 checksum
      uint8_t checksum2_result =
          checksum(rsdp2_ptr, sizeof(rsdp_descriptor_v2));
      if (checksum2_result != 0) {
        klog_warn("RSDP2 checksum is invalid");
      }

      rsdt_paddr = rsdp2_ptr->xsdt_paddr;
      break;
    }
    default:
      klog_err("Unknown RSDP revision number: %d", rsdp_vaddr->revision);
  }

  auto rsdt_vaddr =
      vm::paddr_to_vaddr(reinterpret_cast<acpi_sdt_header_t *>(rsdt_paddr));

  auto acpi_sdt_checksum = checksum(rsdt_vaddr, rsdt_vaddr->length);
  if (acpi_sdt_checksum != 0) {
    klog_warn("ACPI SDT checksum is invalid");
  }

  return {rsdt_vaddr, rsdp_vaddr->revision != 0};
}

struct PACKED acpi_madt_header_t : acpi_sdt_header_t {
  uint32_t local_controller_paddr;
  uint32_t flags;  // 1 = dual legacy pics installed
};

enum acpi_madt_entry_type_t : uint8_t {
  APIC_MADT_PROCESSOR_LOCAL_APIC = 0,
  APIC_MADT_IO_APIC = 1,
  APIC_MADT_INTERRUPT_SOURCE_OVERRIDE = 2,
  APIC_MADT_NMI_SOURCE = 3,
  APIC_MADT_LAPIC_NMI_STRUCTURE = 4,
  APIC_MADT_LAPIC_ADDR_OVERRIDE_STRUCTURE = 5,
  APIC_MADT_IO_SAPIC = 6,
  APIC_MADT_LOCAL_SAPIC = 7,
  APIC_MADT_PLATFORM_INTERRUPT_SOURCES = 8
};

struct PACKED acpi_madt_entry_t {
  acpi_madt_entry_type_t type;
  uint8_t length;
};
STATIC_ASSERT(sizeof(acpi_madt_entry_t) == 2 * sizeof(uint8_t));

struct PACKED acpi_madt_proc_local_entry_t : acpi_madt_entry_t {
  uint8_t processor_id;
  uint8_t apic_id;
  uint32_t flags;  // 1 = processor enabled
};

struct PACKED acpi_madt_io_apic_entry_t : acpi_madt_entry_t {
  uint8_t io_apic_id;
  uint8_t reserved;  // zeroed
  uint32_t io_apic_paddr;
  uint32_t global_interrupt_base;
};

static acpi_madt_header_t *find_madt(const rsdt_result_t &rsdt) {
  assert(rsdt.rsdt_vaddr);

  if (rsdt.is_xsdt) {
    auto entries = (rsdt.rsdt_vaddr->length - sizeof(acpi_sdt_header_t)) /
                   sizeof(uint64_t);
    auto paddr_ptrs = reinterpret_cast<uint64_t *>(&rsdt.rsdt_vaddr[1]);

    for (decltype(entries) i = 0; i < entries; ++i) {
      auto ptr_vaddr = vm::paddr_to_vaddr(
          reinterpret_cast<acpi_sdt_header_t *>(paddr_ptrs[i]));
      klog_debug("Entry %d has signature %s", i, ptr_vaddr->signature);
      if (memcmp(ptr_vaddr->signature, "APIC", 4) == 0) {
        return static_cast<acpi_madt_header_t *>(ptr_vaddr);
      }
    }
  } else {
    auto entries = (rsdt.rsdt_vaddr->length - sizeof(acpi_sdt_header_t)) /
                   sizeof(uint32_t);
    auto paddr_ptrs = reinterpret_cast<uint32_t *>(&rsdt.rsdt_vaddr[1]);

    for (decltype(entries) i = 0; i < entries; ++i) {
      auto ptr_vaddr = vm::paddr_to_vaddr(
          reinterpret_cast<acpi_sdt_header_t *>(paddr_ptrs[i]));
      klog_debug("Entry %d has signature %s", i, ptr_vaddr->signature);
      if (memcmp(ptr_vaddr->signature, "APIC", 4) == 0) {
        return static_cast<acpi_madt_header_t *>(ptr_vaddr);
      }
    }
  }
  klog_err("No MADT found!");
  return nullptr;
}

static void parse_madt(const acpi_madt_header_t *madt_vaddr,
                       acpi::config_t *config) {
  auto madt_base_vaddr = reinterpret_cast<const char *>(madt_vaddr);
  auto madt_entry_vaddr = reinterpret_cast<const char *>(&madt_vaddr[1]);
  while (madt_entry_vaddr < madt_base_vaddr + madt_vaddr->length) {
    auto madt_entry =
        reinterpret_cast<const acpi_madt_entry_t *>(madt_entry_vaddr);

    switch (madt_entry->type) {
      case APIC_MADT_PROCESSOR_LOCAL_APIC: {
        auto entry =
            static_cast<const acpi_madt_proc_local_entry_t *>(madt_entry);
        klog_debug("MADT PL entry (proc id=%d, apic id=%d, enabled=%s)",
                   entry->processor_id, entry->apic_id,
                   entry->flags & 0x1 ? "yes" : "no");
        break;
      }
      case APIC_MADT_IO_APIC: {
        auto entry = static_cast<const acpi_madt_io_apic_entry_t *>(madt_entry);
        auto vaddr = vm::paddr_to_vaddr(entry->io_apic_paddr);
        klog_debug(
            "MADT IOAPIC entry (apic id=%d, paddr=%x, interrupt base=%x)",
            entry->io_apic_id, entry->io_apic_paddr,
            entry->global_interrupt_base);
        config->ioapic_base_vaddr = vaddr;
        break;
      }
      default:
        klog_debug("MADT entry with type %d and length %x", madt_entry->type,
                   madt_entry->length);
    }

    madt_entry_vaddr += madt_entry->length;
  }
}

acpi::config_t acpi::init() {
  config_t config = {};
  auto rsdt = find_rsdt();
  if (rsdt.rsdt_vaddr) {
    klog_debug("Found RSDT at %x", rsdt.rsdt_vaddr);
    auto madt = find_madt(rsdt);
    if (madt) {
      klog_debug("Found MADT at %x", madt);
      parse_madt(madt, &config);
    }
  }
  return config;
}

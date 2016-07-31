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

struct PACKED rsdp_descriptor_v2 {
  rsdp_descriptor first;
  uint32_t length;
  uint64_t xsdt_paddr;
  uint8_t ext_checksum;
  uint8_t reserved[3];
};

// 16-bit pointer to ebda start physical addr right-shifted 4 places
// (so shift left 4 to get actual physical addr after deref)
#define EBDA_PTR_PADDR reinterpret_cast<void *>(0x040e)

static void *get_ebda_addr() {
  auto ebda_ptr_vaddr = vm::paddr_to_vaddr(EBDA_PTR_PADDR);
  auto ebda_paddr = *reinterpret_cast<uint16_t *>(ebda_ptr_vaddr) << 4;
  return vm::paddr_to_vaddr(reinterpret_cast<void *>(ebda_paddr));
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

  auto ebda_addr = reinterpret_cast<char *>(get_ebda_addr());
  klog_debug("Extended BIOS data area is located at %x", ebda_addr);

  auto bios_area_start = reinterpret_cast<char *>(
      vm::paddr_to_vaddr(reinterpret_cast<void *>(0xe0000)));
  auto bios_area_end = reinterpret_cast<char *>(
      vm::paddr_to_vaddr(reinterpret_cast<void *>(0xfffff)));

  auto res = search_for_rsdp(ebda_addr, ebda_addr + 1024);
  if (!res) {
    res = search_for_rsdp(bios_area_start, bios_area_end);
    if (!res) {
      klog_err("No Root System Description Pointer Found!");
    }
  }
  return reinterpret_cast<rsdp_descriptor *>(res);
}

struct acpi_sdt_header_t {
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
  uint8_t checksum_result = checksum(reinterpret_cast<uint8_t *>(rsdp_vaddr),
                                     sizeof(rsdp_descriptor));
  if (checksum_result != 0) {
    klog_warn("RSDP checksum is invalid");
  }

  uint64_t rsdt_paddr = 0;
  switch (rsdp_vaddr->revision) {
    case 0:
      rsdt_paddr = rsdp_vaddr->rsdt_paddr;
      break;
    case 2: {
      auto rsdp2_ptr = reinterpret_cast<rsdp_descriptor_v2 *>(rsdp_vaddr);

      // do rsdp v2 checksum
      uint8_t checksum2_result = checksum(
          reinterpret_cast<uint8_t *>(rsdp2_ptr), sizeof(rsdp_descriptor_v2));
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
      !rsdt_paddr ? nullptr
                  : reinterpret_cast<acpi_sdt_header_t *>(vm::paddr_to_vaddr(
                        reinterpret_cast<void *>(rsdt_paddr)));

  auto acpi_sdt_checksum =
      checksum(reinterpret_cast<uint8_t *>(rsdt_vaddr), rsdt_vaddr->length);
  if (acpi_sdt_checksum != 0) {
    klog_warn("ACPI SDT checksum is invalid");
  }

  return {rsdt_vaddr, rsdp_vaddr->revision != 0};
}

static char *find_madt(const rsdt_result_t &rsdt) {
  assert(rsdt.rsdt_vaddr);

  if (rsdt.is_xsdt) {
    auto entries = (rsdt.rsdt_vaddr->length - sizeof(acpi_sdt_header_t)) /
                   sizeof(uint64_t);
    auto paddr_ptrs = reinterpret_cast<uint64_t *>(&rsdt.rsdt_vaddr[1]);

    for (decltype(entries) i = 0; i < entries; ++i) {
      auto ptr_vaddr = reinterpret_cast<acpi_sdt_header_t *>(
          vm::paddr_to_vaddr(reinterpret_cast<void *>(paddr_ptrs[i])));
      klog_debug("Entry %d has signature %s", i, ptr_vaddr->signature);
      if (memcmp(ptr_vaddr->signature, "APIC", 4) == 0) {
        return reinterpret_cast<char *>(ptr_vaddr);
      }
    }
  } else {
    auto entries = (rsdt.rsdt_vaddr->length - sizeof(acpi_sdt_header_t)) /
                   sizeof(uint32_t);
    auto paddr_ptrs = reinterpret_cast<uint32_t *>(&rsdt.rsdt_vaddr[1]);

    for (decltype(entries) i = 0; i < entries; ++i) {
      auto ptr_vaddr = reinterpret_cast<acpi_sdt_header_t *>(
          vm::paddr_to_vaddr(reinterpret_cast<void *>(paddr_ptrs[i])));
      klog_debug("Entry %d has signature %s", i, ptr_vaddr->signature);
      if (memcmp(ptr_vaddr->signature, "APIC", 4) == 0) {
        return reinterpret_cast<char *>(ptr_vaddr);
      }
    }
  }
  klog_err("No MADT found!");
  return nullptr;
}

void acpi::init() {
  auto rsdt = find_rsdt();
  if (rsdt.rsdt_vaddr) {
    klog_debug("Found RSDT at %x", rsdt.rsdt_vaddr);
    auto madt = find_madt(rsdt);
    klog_debug("Found MADT at %x", madt);
  }
}

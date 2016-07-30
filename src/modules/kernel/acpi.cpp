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

static rsdp_descriptor *find_rsdp_ptr() {
  // places to search (only 16-byte boundaries):
  //  - ebda_addr to ebda_addr + 1024
  //  - 0xE0000 to 0xFFFFF

  auto ebda_addr = reinterpret_cast<char *>(get_ebda_addr());
  klog_debug("Extended BIOS data area is located at %x", ebda_addr);

  auto bios_area_start = reinterpret_cast<char *>(
      vm::paddr_to_vaddr(reinterpret_cast<void *>(0xe0000)));
  auto bios_area_end = reinterpret_cast<char *>(
      vm::paddr_to_vaddr(reinterpret_cast<void *>(0xfffff)));

  auto res1 = search_for_rsdp(ebda_addr, ebda_addr + 1024);
  auto res2 = search_for_rsdp(bios_area_start, bios_area_end);
  if (!res1 && !res2) {
    klog_err("No Root System Description Pointer Found!");
  }
  return reinterpret_cast<rsdp_descriptor *>(res1 ? res1 : res2);
}

static char *find_rsdt_ptr() {
  // first find the rsdp, then parse it to find a pointer to the rsdt
  auto rsdp_ptr = find_rsdp_ptr();

  uint8_t checksum_result =
      checksum(reinterpret_cast<uint8_t *>(rsdp_ptr), sizeof(rsdp_descriptor));
  assert(checksum_result == 0);

  uint64_t rsdt_paddr = 0;
  switch (rsdp_ptr->revision) {
    case 0:
      rsdt_paddr = rsdp_ptr->rsdt_paddr;
      break;
    case 2: {
      auto rsdp2_ptr = reinterpret_cast<rsdp_descriptor_v2 *>(rsdp_ptr);

      uint8_t checksum2_result = checksum(
          reinterpret_cast<uint8_t *>(rsdp2_ptr), sizeof(rsdp_descriptor_v2));
      assert(checksum2_result == 0);

      rsdt_paddr = rsdp2_ptr->xsdt_paddr;
      break;
    }
    default:
      klog_err("Unknown RSDP revision number: %d", rsdp_ptr->revision);
  }

  return !rsdt_paddr ? nullptr : reinterpret_cast<char *>(vm::paddr_to_vaddr(
                                     reinterpret_cast<void *>(rsdt_paddr)));
}

void acpi::init() {
  auto rsdt_ptr = find_rsdt_ptr();
  klog_debug("Found RSDT at %x", rsdt_ptr);
}

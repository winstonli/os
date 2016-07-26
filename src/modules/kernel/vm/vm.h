#pragma once

#include <cstdint>

#include <boot/multiboot_info.h>
#include <vm/page_table.h>

extern "C" void *link_kern_start;
extern "C" void *link_kern_end;

class vm {
  static constexpr uint64_t pgsz_4k = 0x1000;

  static constexpr uint64_t pgsz_2m = page_table::num_entries * pgsz_4k;

  static constexpr uint64_t pgsz_1g = page_table::num_entries * pgsz_2m;

  static constexpr uint64_t kernel_offset = 0xffff'ffff'8000'0000;

 public:

  static void init(const multiboot_info &multiboot);

  template <typename T>
  static T *align_up(T *addr, int64_t sz) {
    uintptr_t iaddr = reinterpret_cast<uintptr_t>(addr);
    uintptr_t rem = iaddr % sz;
    if (rem == 0) {
      return addr;
    }
    return reinterpret_cast<T *>(iaddr - rem + sz);
  }

  static void *paddr_to_kvaddr(void *paddr);

  static void *kvaddr_to_paddr(void *paddr);

  static void *align_up_4k(void *addr);

  static void *align_up_2m(void *addr);

  static void *align_up_1g(void *addr);
};

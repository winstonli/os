#pragma once

#include <vm/page_table.h>

class vm {

  static constexpr uint64_t direct_virtual_offset = 0xffff'8800'0000'0000;
  static constexpr uint64_t pt_offset = 0xffff'ea00'0000'0000;
  static constexpr uint64_t kernel_offset = 0xffff'ffff'8000'0000;

public:

  static constexpr uint64_t pgsz_4k = 0x1000;
  static constexpr uint64_t pgsz_2m = page_table::num_entries * pgsz_4k;
  static constexpr uint64_t pgsz_1g = page_table::num_entries * pgsz_2m;
  static constexpr uint64_t pgsz_512g = page_table::num_entries * pgsz_1g;

  template<typename T>
  static T *align_up(T *addr, uintptr_t sz) {
    uintptr_t iaddr = reinterpret_cast<uintptr_t>(addr);
    uintptr_t rem = iaddr % sz;
    if (rem == 0) {
      return addr;
    }
    return reinterpret_cast<T *>(iaddr - rem + sz);
  }

  template<typename T>
  static T *align_down(T *addr, uintptr_t sz) {
    uintptr_t num = reinterpret_cast<uintptr_t>(addr) / sz;
    return reinterpret_cast<char *>(num * sz);
  }

  /*
     These functions translate between physical addresses and
     addresses in the direct virtual mapping (direct_virtual_offset).
   */

  template<typename T>
  static T *paddr_to_vaddr(T *paddr) {
    return static_cast<T*>(paddr_to_vaddr(static_cast<void*>(paddr)));
  }

  template<typename T>
  static T *vaddr_to_paddr(T *vaddr) {
    return static_cast<T*>(vaddr_to_paddr(static_cast<void*>(vaddr)));
  }

  /*
     These functions translate between physical addresses and
     addresses in the page table space (pt_offset).
   */

  static void *paddr_to_ptvaddr(void *paddr);

  static void *ptvaddr_to_paddr(void *ptvaddr);

  /*
     These functions translate between physical addresses and
     addresses in the kernel .text space (kernel_offset).
   */

  static void *paddr_to_kvaddr(void *paddr);

  static void *kvaddr_to_paddr(void *kvaddr);

  template<typename T>
  static T *align_up_4k(T *addr) {
    return align_up(addr, pgsz_4k);
  }

  template<typename T>
  static T *align_up_2m(T *addr) {
    return align_up(addr, pgsz_2m);
  }

  template<typename T>
  static T *align_up_1g(T *addr) {
    return align_up(addr, pgsz_1g);
  }

  template<typename T>
  static T *align_up_512g(T *addr) {
    return align_up(addr, pgsz_512g);
  }

  template<typename T>
  static T *align_down_4k(T *addr) {
    return align_down(addr, pgsz_4k);
  }

  template<typename T>
  static T *align_down_2m(T *addr) {
    return align_down(addr, pgsz_2m);
  }

  template<typename T>
  static T *align_down_1g(T *addr) {
    return align_down(addr, pgsz_1g);
  }

  template<typename T>
  static T *align_down_512g(T *addr) {
    return align_down(addr, pgsz_512g);
  }

  static uintptr_t num_frames(uintptr_t num_bytes, uintptr_t sz);

  static uintptr_t num_frames_2m(uintptr_t num_bytes);

};

template<>
void *vm::paddr_to_vaddr<void>(void *paddr);

template<>
void *vm::vaddr_to_paddr<void>(void *vaddr);


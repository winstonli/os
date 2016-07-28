#include "pager.h"

#include <algorithm>

#include <common/common.h>

#include <log.h>
#include <register.h>

#include <vm/vm.h>

pager::pager(frame_pool &frpool) : pt(find_page_table()), frpool(frpool) {
  init_direct_mapping();
}

void pager::init_direct_mapping() {
  void *maxmem = frpool.get_maxmem();
  klog("max addressible memory: %x", maxmem);
  uintptr_t total_2m_pages =
      reinterpret_cast<uintptr_t>(vm::align_up_2m(maxmem)) / vm::pgsz_2m;
  uintptr_t total_1g_pages =
      reinterpret_cast<uintptr_t>(vm::align_up_1g(maxmem)) / vm::pgsz_1g;
  uintptr_t total_512g_pages =
      reinterpret_cast<uintptr_t>(vm::align_up_512g(maxmem)) / vm::pgsz_512g;
  assertf(total_512g_pages <= 128, "too much memory to map: %x", maxmem);
  klog("total_512g_pages = %x", total_512g_pages);
  klog("total_1g_pages = %x", total_1g_pages);
  klog("total_2m_pages = %x", total_2m_pages);
  klog("pt4[0x110] = %x", pt->e4_arr[0x110]);
  klog("pt3[0] = %x", pt->e4_arr[0x110].get_pdp()[0]);
  pdpe *pt3 = pt->e4_arr[0x110].get_pdp();
  pde *pt2_kern0 = pt3[0].get_pd();
  for (auto i = 0u; i < std::min(0x200ul, total_2m_pages); ++i) {
    pt2_kern0[i].set(vm::pgsz_2m * i | pde::bit_p | pde::bit_rw | pde::bit_ps);
  }
  for (auto i = 1u; i < total_1g_pages; ++i) {
  }
  *static_cast<int64_t *>(
      vm::paddr_to_vaddr(reinterpret_cast<void *>(0x1ff * 0x200000))) = 15;
  // panic();
}

page_table *pager::find_page_table() {
  return static_cast<page_table *>(
      vm::paddr_to_kvaddr(reinterpret_cast<void *>(get_cr3())));
}

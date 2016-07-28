#include "pager.h"

#include <algorithm>

#include <common/common.h>

#include <log.h>
#include <register.h>

#include <vm/vm.h>

pager::pager(frame_pool &pool) : pt(find_page_table()), frpool(pool) {
  init_direct_mapping();
}

void pager::init_direct_mapping() {
  void *maxmem = frpool.get_maxmem();
  assert(maxmem != nullptr);
  uintptr_t total_2m_pages =
      reinterpret_cast<uintptr_t>(vm::align_up_2m(maxmem)) / vm::pgsz_2m;
  uintptr_t total_1g_pages =
      reinterpret_cast<uintptr_t>(vm::align_up_1g(maxmem)) / vm::pgsz_1g;
  uintptr_t total_512g_pages =
      reinterpret_cast<uintptr_t>(vm::align_up_512g(maxmem)) / vm::pgsz_512g;
  assertf(total_512g_pages <= 128, "too much memory to map: %x", maxmem);
  pdpe *pt3 = pt->e4_arr[0x110].get_pdp();
  *static_cast<int64_t *>(
      vm::paddr_to_vaddr(reinterpret_cast<void *>(0x1601000))) = 15;
  /* Let's map everything in the first 512 GiB because we have the first
     L3 page table and L2 page table allocated already */
  if (total_1g_pages == 1) {
    return;
  }
  /* Map the rest */
  pde *first_1g_pd_arr =
      static_cast<pde *>(vm::paddr_to_vaddr(frpool.falloc_perm_2m()));
  uint64_t num_2m_pages = 0;
  for (auto i = 0u; i < std::min(0x200ul, total_1g_pages); ++i) {
    pde *current_pd = first_1g_pd_arr + i * 0x200;
    for (auto j = 0; j < 0x200 && num_2m_pages < total_2m_pages;
         ++j, ++num_2m_pages) {
      current_pd[j].set(num_2m_pages * vm::pgsz_2m | pde::bit_p | pde::bit_rw |
                        pde::bit_ps);
    }
    pt3[i].set(reinterpret_cast<uintptr_t>(vm::vaddr_to_paddr(current_pd)) |
               pdpe::bit_p | pdpe::bit_rw);
  }
}

page_table *pager::find_page_table() {
  return static_cast<page_table *>(
      vm::paddr_to_kvaddr(reinterpret_cast<void *>(get_cr3())));
}

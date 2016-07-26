#include "page_table.h"

#include <common/common.h>

#include <log.h>
#include <register.h>
#include <terminal.h>
#include <util/fixedsize_vector.h>
#include <vm/vm.h>

DATA page_table *page_table::ptr;

void page_table::init() {
  klog_debug("Initializing real page table");
  uint64_t cr3 = get_cr3();
  void *pt = vm::paddr_to_kvaddr(reinterpret_cast<void *>(cr3));

  klog_debug("cr3: %x", cr3);
  ptr = static_cast<page_table *>(pt);
  klog_debug("page_table: %x", ptr);
  const auto &last_pml4e = ptr->e4_arr[511];
  const auto last_pdp = last_pml4e.get_pdp();
  klog_debug("last pml4 entry pdp kvaddr: %x", last_pdp);
  const auto pd = last_pdp[510].get_pd();
  klog_debug("second last pd: %x", pd);
  klog_debug("entry: %x", *reinterpret_cast<const uint64_t *>(pd));
  // pd->ps = true;
  klog_debug("ps: %x", !pd->has_pt());
  klog_debug("pt: %x", pd->get_pt());

  klog_debug("link_kern_end: %x", &link_kern_end);
  klog_debug("link_kern_end aligned: %x", vm::align_up_2m(&link_kern_end));
  klog_debug("link_kern_end p aligned: %x",
             vm::align_up_2m(vm::kvaddr_to_paddr(&link_kern_end)));
}

void *page_table::get_entry_paddr(const void *entry_ptr) {
  return reinterpret_cast<void *>(*static_cast<const uint64_t *>(entry_ptr) &
                                  base_paddr_mask);
}

void *page_table::get_entry_kvaddr(const void *entry_ptr) {
  return vm::paddr_to_kvaddr(get_entry_paddr(entry_ptr));
}

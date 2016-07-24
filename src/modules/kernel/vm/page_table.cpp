#include "page_table.h"

#include <common/common.h>

#include <register.h>
#include <terminal.h>
#include <vm/vm.h>

DATA page_table *page_table::ptr;

void page_table::init() {
  terminal_printf("Initializing real page table\n");
  uint64_t cr3 = get_cr3();
  void *pt = vm::paddr_to_kvaddr(reinterpret_cast<void *>(cr3));

  terminal_printf("cr3: %x\n", cr3);
  ptr = static_cast<page_table *>(pt);
  terminal_printf("page_table: %x\n", ptr);
  const auto &last_pml4e = ptr->e4_arr[511];
  const auto last_pdp = last_pml4e.get_pdp();
  terminal_printf("last pml4 entry pdp kvaddr: %x\n", last_pdp);
  const auto pd = last_pdp[510].get_pd();
  terminal_printf("second last pd: %x\n", pd);
  terminal_printf("entry: %x\n", *(uint64_t *)pd);
  // pd->ps = true;
  terminal_printf("ps: %x\n", !pd->has_pt());
  terminal_printf("pt: %x\n", pd->get_pt());

  terminal_printf("link_kern_end: %x\n", &link_kern_end);
  terminal_printf("link_kern_end aligned: %x\n",
                  vm::align_up_2m(&link_kern_end));
}

void *page_table::get_entry_paddr(const void *entry_ptr) {
  return reinterpret_cast<void *>(*static_cast<const uint64_t *>(entry_ptr) &
                                  base_paddr_mask);
}

void *page_table::get_entry_kvaddr(const void *entry_ptr) {
  return vm::paddr_to_kvaddr(get_entry_paddr(entry_ptr));
}

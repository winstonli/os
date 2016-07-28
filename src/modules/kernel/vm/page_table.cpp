#include "page_table.h"

#include <vm/vm.h>

void *page_table::get_entry_paddr(const void *entry_ptr) {
  return reinterpret_cast<void *>(*static_cast<const uint64_t *>(entry_ptr) &
                                  base_paddr_mask);
}

void *page_table::get_entry_vaddr(const void *entry_ptr) {
  return vm::paddr_to_vaddr(get_entry_paddr(entry_ptr));
}

void page_table::set_entry_paddr(void *entry_ptr, void *paddr) {
  *static_cast<uint64_t *>(entry_ptr) =
      (reinterpret_cast<uint64_t>(paddr) & base_paddr_mask);
}

void page_table::set_entry_vaddr(void *entry_ptr, void *vaddr) {
  set_entry_paddr(entry_ptr, vm::vaddr_to_paddr(vaddr));
}

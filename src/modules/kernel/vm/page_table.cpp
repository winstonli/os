#include "page_table.h"

#include <register.h>
#include <terminal.h>
#include <vm/vm.h>

[[gnu::section(".data")]] page_table *page_table::ptr;

void page_table::init() {
  terminal_printf("Initializing real page table\n");
  uint64_t cr3 = get_cr3();
  void *pt = vm::paddr_to_kvaddr(reinterpret_cast<void *>(cr3));

  terminal_printf("cr3: %x\n", cr3);
  ptr = static_cast<page_table *>(pt);
  terminal_printf("page_table: %x\n", ptr);
  terminal_printf("last pml4 entry kvaddr: %x\n",
                  ptr->e4_arr[511].get_pdpe_base());
}

#include "page_table.h"

#include <register.h>
#include <terminal.h>
#include <vm/vm.h>

[[gnu::section(".data")]] page_table *page_table::ptr = nullptr;

void page_table::init() {
  terminal_printf("Initializing real page table\n");
  uint64_t cr3 = get_cr3();
  void *pt = vm::paddr_to_kaddr(reinterpret_cast<void *>(cr3));

  terminal_printf("cr3: %x\n", cr3);
  terminal_printf("vaddr: %x\n",
                  vm::paddr_to_kaddr(reinterpret_cast<void *>(cr3)));
  ptr = static_cast<page_table *>(pt);
  terminal_printf("page_table: %x\n", ptr);
}

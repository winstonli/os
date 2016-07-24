#pragma once

#include <common/common.h>

#include <vm/pml4e.h>

class PACKED page_table {

  static constexpr uint64_t base_paddr_mask = 0x000f'ffff'ffff'f000;

  static page_table *ptr;

public:
  static constexpr int num_entries = 512;

private:
  pml4e e4_arr[num_entries];

public:
  /* The page table is already set up, and cr3 contains its paddr.
     All we want to do is take cr3, translate it to vaddr, and set pt_ptr */
  static void init();

  /* Gets the physical address out of any page table entry. They're always
     at bits 51-12 */
  static void *get_entry_paddr(const void *entry_ptr);

  /* Same as get_entry_paddr, except for the kvaddr. */
  static void *get_entry_kvaddr(const void *entry_ptr);
};

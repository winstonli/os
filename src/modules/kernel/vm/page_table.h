#pragma once

#include <common/common.h>

#include <vm/pml4e.h>

struct PACKED page_table {

private:

  static constexpr uint64_t base_paddr_mask = 0x000f'ffff'ffff'f000;

 public:

  static constexpr int num_entries = 512;

  pml4e e4_arr[num_entries];

public:

  /* Gets the physical address out of any page table entry. They're always
     at bits 51-12 */
  static void *get_entry_paddr(const void *entry_ptr);

  /* Same as get_entry_paddr, except for the vaddr. */
  static void *get_entry_vaddr(const void *entry_ptr);

  static void set_entry_paddr(void *entry_ptr, void *paddr);

  static void set_entry_vaddr(void *entry_ptr, void *vaddr);

};

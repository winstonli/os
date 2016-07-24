#pragma once

#include <common/common.h>

#include <modules/kernel/vm/pml4e.h>

class PACKED page_table {

  static constexpr int num_pages = 512;

  pml4e e4_arr[num_pages];

  static page_table *ptr;

public:
  /* The page table is already set up, and cr3 contains its paddr.
     All we want to do is take cr3, translate it to vaddr, and set pt_ptr */
  static void init();
};

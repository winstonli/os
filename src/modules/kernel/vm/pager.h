#pragma once

#include <vm/frame_pool.h>
#include <vm/page_table.h>

class pager {

  page_table *pt;
  frame_pool &frpool;

public:

  /*
     The page table is already set up, and cr3 contains its paddr.
     All we want to do is take cr3, translate it to its direct vaddr,
     and set page_table::ptr
   */
  pager(frame_pool &frpool);

private:

  page_table *find_page_table();
  /*
     Maps the 64 TB direct mapping at 0xffff'8800'0000'0000.

     This starts at the 512 GiB PT4 entry at offset 0x110, which is mapped to a
     1 GiB PT3 table.

     The PT3 table at offset 0x0 is mapped to a 2 MiB PT2.

     PT2 has some of its first mappings as identity offsets, and is already
     used in mapping the first several pages from the kernel text mapping,
     which starts at phys 0.
   */
  void init_direct_mapping();

};

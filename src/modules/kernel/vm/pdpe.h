#pragma once

#include <common/common.h>

#include <vm/pde.h>

/*

   Class representing a level 3 page directory pointer entry (pdpe).

 */
class PACKED pdpe {
  /*
     Present (P) bit:
     true = Page loaded in physical memory. (Default)
     false = Page not loaded in physical memory.
   */
  bool p : 1;
  /*
     Read/Write (R/W) bit:
     true = Page is read/write. (Default)
     false = Page is read-only.
   */
  bool rw : 1;
  /*
     User/Supervisor (U/S) bit:
     true = Page has user and supervisor access.
     false = Page only has supervisor access. (Default)
  */
  bool us : 1;
  /*
     Page-Level Writethrough (PWT) bit:
     true = Page uses writethrough caching.
     false = Page uses writeback caching. (Default)
   */
  bool pwt : 1;
  /*
     Page-Level Cache Disable (PCD) bit:
     true = Page not cachable.
     false = Page is cachable. (Default)
   */
  bool pcd : 1;
  /*
     Accessed (A) bit:
     true = Page was read from or written to.
     false = Page was never read from or written to. (Default)

     The CPU can only set this bit. It is up to the kernel to clear it.
   */
  bool a : 1;
  /*
     Only valid if the Page Size (PS) bit is set.

     Dirty (D) bit:
     true = Page was written to.
     false = Page was never written to. (Default).

     The CPU can only set this bit. It is up to the kernel to clear it.
   */
  bool d : 1;
  /*
     Page Size (PS) bit:
     true = This entry is a a leaf entry, and points to a 2 MiB page.
     false = This entry points to a level 1 page table.
   */
  bool ps : 1;
  /*
     Only valid if the Page Size (PS) bit is set.

     Global (G) bit
   */
  bool g : 1;
  /*
     Available to Software (AVL)

     Not interpreted by the CPU.
   */
  uint64_t avl : 3;
  /*
     The base physical address of a level 2 page directory or 1 GiB frame,
     depending on whether the Page Size (PS) bit is set.

     If the PS bit is set, the bottom bit (bit 12 in the entry) is the PAT bit,
     and bits 1-28 (bits 13-30 in the entry) MUST be zeroed or there will be a
     page fault. It's recommended just to zero bits 0-28 (12-30 in the entry).
   */
  uint64_t pd_base_paddr : 40;
  uint64_t available : 11;
  /*
     No Execute (NX) bit:
     true = Execution not allowed on this page.
     false = Execution allowed on this page.
   */
  bool nx : 1;

 public:
  pde *get_pd() const;
};

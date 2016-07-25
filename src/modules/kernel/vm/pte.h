#pragma once

#include <common/common.h>

/*

   Class representing a level 1 page table entry (pte).

 */
class PACKED pte {
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
     Dirty (D) bit:
     true = Page was written to.
     false = Page was never written to. (Default).

     The CPU can only set this bit. It is up to the kernel to clear it.
   */
  bool d : 1;
  /*
     Page-Attribute Table (PAT) bit
   */
  bool pat : 1;
  /*
     Global (G) bit
   */
  bool g : 1;
  /*
     Available to Software (AVL)

     Not interpreted by the CPU.
   */
  uint64_t avl : 3;
  /*
     The base address of the physical page frame.
   */
  uint64_t base_paddr : 40;
  uint64_t available : 11;
  /*
     No Execute (NX) bit:
     true = Execution not allowed on this page.
     false = Execution allowed on this page.
   */
  bool nx : 1;

 public:
  void *get_page_paddr() const;
};

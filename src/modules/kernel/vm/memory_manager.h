#pragma once

#include <boot/multiboot_info.h>
#include <vm/frame_pool.h>
#include <vm/pager.h>

class memory_manager {

  frame_pool frpool;
  pager p;

public:

  memory_manager(
      const multiboot_info &multiboot,
      void *start_mod_start,
      void *start_mod_end,
      void *kern_mod_start,
      void *kern_mod_end
  );

};

#pragma once

#include <boot/multiboot_info.h>
#include <vm/memory_manager.h>

class kernel {

  memory_manager mem;

public:

  kernel(
      const multiboot_info &multiboot,
      void *start_mod_start,
      void *start_mod_end,
      void *kern_mod_start,
      void *kern_mod_end
  );

  kernel(const kernel &) = delete;
  kernel &operator=(const kernel &) = delete;

  kernel(kernel &&) = delete;
  kernel &operator=(kernel &&) = delete;

  ~kernel() = default;

};

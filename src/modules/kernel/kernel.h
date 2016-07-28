#pragma once

#include <boot/multiboot_info.h>
#include <vm/vm.h>

class kernel {

  vm vm;

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

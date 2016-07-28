#include "kernel.h"

kernel::kernel(const multiboot_info &multiboot, void *start_mod_start,
               void *start_mod_end, void *kern_mod_start, void *kern_mod_end)
    : mem(multiboot, start_mod_start, start_mod_end, kern_mod_start,
          kern_mod_end) {}

#include "memory_manager.h"

memory_manager::memory_manager(const multiboot_info &multiboot,
                               void *start_mod_start, void *start_mod_end,
                               void *kern_mod_start, void *kern_mod_end)
    : frpool(multiboot, start_mod_start, start_mod_end, kern_mod_start,
             kern_mod_end),
      p(frpool) {
  auto mem = frpool.get_avail_mem();
  if (mem < vm::pgsz_1g) {
    klog("Available memory: %d MiB (%zd bytes)", mem / 0x100000, mem);
  } else {
    klog("Available memory: %d GiB (%zd bytes)", mem / 0x40000000, mem);
  }
}

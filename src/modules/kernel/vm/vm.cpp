#include "vm.h"

#include <vm/frame_pool.h>
#include <vm/pager.h>

vm::vm(const multiboot_info &multiboot, void *start_mod_start,
       void *start_mod_end, void *kern_mod_start, void *kern_mod_end)
    : frpool(multiboot, start_mod_start, start_mod_end, kern_mod_start,
             kern_mod_end),
      p(frpool) {}

void *vm::paddr_to_kvaddr(void *paddr) {
  return static_cast<char *>(paddr) + kernel_offset;
}

void *vm::kvaddr_to_paddr(void *kvaddr) {
  return static_cast<char *>(kvaddr) - kernel_offset;
}

void *vm::paddr_to_vaddr(void *paddr) {
  return static_cast<char *>(paddr) + direct_virtual_offset;
}

void *vm::vaddr_to_paddr(void *vaddr) {
  return static_cast<char *>(vaddr) - direct_virtual_offset;
}

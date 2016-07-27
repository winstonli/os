#include "vm.h"

#include <assert.h>
#include <util/fixedsize_vector.h>
#include <vm/frame_pool.h>

void vm::init(const multiboot_info &multiboot, void *start_mod_start,
              void *start_mod_end) {
  page_table::init();
  frame_pool::init(multiboot, start_mod_start, start_mod_end);
}

void *vm::paddr_to_kvaddr(void *paddr) {
  return static_cast<char *>(paddr) + kernel_offset;
}

void *vm::kvaddr_to_paddr(void *kaddr) {
  return static_cast<char *>(kaddr) - kernel_offset;
}

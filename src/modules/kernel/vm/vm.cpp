#include "vm.h"

void *vm::paddr_to_kvaddr(void *paddr) {
  return static_cast<char *>(paddr) + kernel_offset;
}

void *vm::kvaddr_to_paddr(void *kaddr) {
  return static_cast<char *>(kaddr) - kernel_offset;
}

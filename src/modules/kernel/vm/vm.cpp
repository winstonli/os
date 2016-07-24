#include "vm.h"

void *vm::paddr_to_kaddr(void *paddr) {
  return static_cast<char *>(paddr) + kernel_offset;
}

void *vm::kaddr_to_paddr(void *kaddr) {
  return static_cast<char *>(kaddr) - kernel_offset;
}

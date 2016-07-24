#include "vm.h"

void *vm::align_up(void *addr, int64_t pgsz) {
  uintptr_t iaddr = reinterpret_cast<uintptr_t>(addr);
  uintptr_t rem = iaddr % pgsz;
  if (rem == 0) {
    return addr;
  }
  return reinterpret_cast<void *>(iaddr - rem + pgsz);
}

void *vm::paddr_to_kvaddr(void *paddr) {
  return static_cast<char *>(paddr) + kernel_offset;
}

void *vm::kvaddr_to_paddr(void *kaddr) {
  return static_cast<char *>(kaddr) - kernel_offset;
}

void *vm::align_up_4k(void *addr) {
  return align_up(addr, pgsz_4k);
}

void *vm::align_up_2m(void *addr) {
  return align_up(addr, pgsz_2m);
}

void *vm::align_up_1g(void *addr) {
  return align_up(addr, pgsz_1g);
}

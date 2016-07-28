#include "vm.h"

void *vm::paddr_to_vaddr(void *paddr) {
  return static_cast<char *>(paddr) + direct_virtual_offset;
}

void *vm::vaddr_to_paddr(void *vaddr) {
  return static_cast<char *>(vaddr) - direct_virtual_offset;
}

void *vm::paddr_to_ptvaddr(void *paddr) {
  return static_cast<char *>(paddr) + pt_offset;
}

void *vm::ptvaddr_to_paddr(void *ptvaddr) {
  return static_cast<char *>(ptvaddr) - pt_offset;
}

void *vm::paddr_to_kvaddr(void *paddr) {
  return static_cast<char *>(paddr) + kernel_offset;
}

void *vm::kvaddr_to_paddr(void *kvaddr) {
  return static_cast<char *>(kvaddr) - kernel_offset;
}

uintptr_t vm::num_frames(uintptr_t num_bytes, uintptr_t sz) {
  void *aligned = align_up(reinterpret_cast<void *>(num_bytes), sz);
  return reinterpret_cast<uintptr_t>(aligned) / sz;
}

uintptr_t vm::num_frames_2m(uintptr_t num_bytes) {
  return num_frames(num_bytes, pgsz_2m);
}

#include "vm.h"

template <>
void *vm::paddr_to_vaddr(void *paddr) {
  if (paddr == nullptr) return nullptr;
  return static_cast<char *>(paddr) + direct_virtual_offset;
}

template <>
void *vm::vaddr_to_paddr(void *vaddr) {
  if (vaddr == nullptr) return nullptr;
  return static_cast<char *>(vaddr) - direct_virtual_offset;
}

uintptr_t vm::paddr_to_vaddr(uintptr_t paddr) {
  return reinterpret_cast<uintptr_t>(
      paddr_to_vaddr(reinterpret_cast<void *>(paddr)));
}

uintptr_t vm::vaddr_to_paddr(uintptr_t vaddr) {
  return reinterpret_cast<uintptr_t>(
      vaddr_to_paddr(reinterpret_cast<void *>(vaddr)));
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

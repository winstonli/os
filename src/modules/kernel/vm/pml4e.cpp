#include "pml4e.h"

#include <vm/vm.h>

void *pml4e::get_pdpe_base_paddr() const {
  return reinterpret_cast<void *>(*reinterpret_cast<const uint64_t *>(this) &
                                  base_paddr_mask);
}

void *pml4e::get_pdpe_base_kvaddr() const {
  return vm::paddr_to_kvaddr(get_pdpe_base_paddr());
}

pdpe *pml4e::get_pdpe_base() const {
  return static_cast<pdpe *>(get_pdpe_base_kvaddr());
}

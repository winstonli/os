#include "pdpe.h"

#include <vm/page_table.h>

void pdpe::set(uint64_t bits) { *reinterpret_cast<uint64_t *>(this) = bits; }

void pdpe::set_pd(pde *pd) {
  page_table::set_entry_vaddr(static_cast<void *>(this),
                              static_cast<void *>(pd));
}

pde *pdpe::get_pd() const {
  return static_cast<pde *>(
      page_table::get_entry_vaddr(static_cast<const void *>(this)));
}

#include "pde.h"

#include <vm/page_table.h>

void pde::set(uint64_t bits) { *reinterpret_cast<uint64_t *>(this) = bits; }

bool pde::has_pt() const { return !ps; }

pte *pde::get_pt() const {
  return static_cast<pte *>(
      page_table::get_entry_vaddr(static_cast<const void *>(this)));
}

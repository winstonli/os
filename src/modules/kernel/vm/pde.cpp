#include "pde.h"

#include <vm/page_table.h>

bool pde::has_pt() const {
  return !ps;
}

pte *pde::get_pt() const {
  return static_cast<pte *>(
      page_table::get_entry_kvaddr(static_cast<const void *>(this)));
}

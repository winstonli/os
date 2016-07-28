#include "pdpe.h"

#include <vm/page_table.h>

pde *pdpe::get_pd() const {
  return static_cast<pde *>(
      page_table::get_entry_vaddr(static_cast<const void *>(this)));
}

#include "pte.h"

#include <vm/page_table.h>

void *pte::get_page_paddr() const {
  return page_table::get_entry_paddr(static_cast<const void *>(this));
}

#include "pml4e.h"

#include <vm/page_table.h>
#include <vm/vm.h>

pdpe *pml4e::get_pdp() const {
  return static_cast<pdpe *>(
      page_table::get_entry_kvaddr(static_cast<const void *>(this)));
}

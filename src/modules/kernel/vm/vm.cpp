#include "vm.h"

#include <assert.h>
#include <util/fixedsize_vector.h>

void vm::init(const multiboot_info &multiboot) {
  page_table::init();
  fixedsize_vector<multiboot_mmap_entry, 32> v;
  multiboot.get_memory_map(v);
  for (auto i = 0u; i < v.size(); ++i) {
    const multiboot_mmap_entry &e = v[i];
    klog_debug("Memchunk addr: %x", e.addr);
    klog_debug("Memchunk len: %x", e.len);
  }
  // panic();
}

void *vm::paddr_to_kvaddr(void *paddr) {
  return static_cast<char *>(paddr) + kernel_offset;
}

void *vm::kvaddr_to_paddr(void *kaddr) {
  return static_cast<char *>(kaddr) - kernel_offset;
}

void *vm::align_up_4k(void *addr) { return align_up(addr, pgsz_4k); }

void *vm::align_up_2m(void *addr) { return align_up(addr, pgsz_2m); }

void *vm::align_up_1g(void *addr) { return align_up(addr, pgsz_1g); }

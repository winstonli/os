#include "multiboot_info.h"

#include <array>

#include <common/common.h>

#include <assert.h>
#include <log.h>
#include <terminal.h>
#include <vm/vm.h>

DATA multiboot_info *multiboot_info::ptr = nullptr;

multiboot_tag_basic_meminfo *multiboot_info::get_meminfo() {
  char *curr_ptr = reinterpret_cast<char *>(this) + 8;
  multiboot_tag *curr = reinterpret_cast<multiboot_tag *>(curr_ptr);
  while (curr->type != 4) {
    curr_ptr += curr->size;
    curr_ptr = vm::align_up(curr_ptr, 8);
    curr = reinterpret_cast<multiboot_tag *>(curr_ptr);
    assert(curr->type != 0);
  }
  return reinterpret_cast<multiboot_tag_basic_meminfo *>(curr);
}

multiboot_tag_mmap *multiboot_info::get_mmap() {
  char *curr_ptr = reinterpret_cast<char *>(this) + 8;
  multiboot_tag *curr = reinterpret_cast<multiboot_tag *>(curr_ptr);
  while (curr->type != 6) {
    curr_ptr += curr->size;
    curr_ptr = vm::align_up(curr_ptr, 8);
    curr = reinterpret_cast<multiboot_tag *>(curr_ptr);
    assert(curr->type != 0);
  }
  return reinterpret_cast<multiboot_tag_mmap *>(curr);
}

void multiboot_info::init(void *multiboot_ptr) {
  ptr = static_cast<multiboot_info *>(multiboot_ptr);
}

uint32_t multiboot_info::get_num_mmap_entries(multiboot_tag_mmap *mmap) {
  return (mmap->size - sizeof(multiboot_tag_mmap)) / mmap->entry_size;
}

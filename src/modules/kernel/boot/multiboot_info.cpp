#include "multiboot_info.h"

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
    if (curr->type == 0) {
      panic();
    }
  }
  return nullptr;
}

void multiboot_info::init(void *multiboot_ptr) {
  ptr = static_cast<multiboot_info *>(multiboot_ptr);
  klog_debug("Multiboot total_size = %x", ptr->total_size);
  klog_debug("Multiboot reserved = %x", ptr->reserved);
}

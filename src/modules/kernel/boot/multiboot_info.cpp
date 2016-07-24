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
  klog_debug("Multiboot total_size = %x", ptr->total_size);
  klog_debug("Multiboot reserved = %x", ptr->reserved);
  multiboot_tag_basic_meminfo *meminfo = ptr->get_meminfo();
  klog_debug("Meminfo type = %x", meminfo->type);
  klog_debug("Meminfo size = %x", meminfo->size);
  klog_debug("Meminfo mem_lower = %x", meminfo->mem_lower);
  klog_debug("Meminfo mem_upper = %x", meminfo->mem_upper);
  multiboot_tag_mmap *mmap = ptr->get_mmap();
  klog_debug("Mmap type = %x", mmap->type);
  klog_debug("Mmap size = %x", mmap->size);
  klog_debug("Mmap entry_size = %x", mmap->entry_size);
  klog_debug("Mmap entry_version = %x", mmap->entry_version);
  uint32_t num_mmap_entries = get_num_mmap_entries(mmap);
  klog_debug("Num mmap entries: %x", num_mmap_entries);
  multiboot_mmap_entry *entries = mmap->entries;
  for (uint32_t i = 0; i < num_mmap_entries; ++i) {
    klog_debug("Mmap entry %d/%d", i, num_mmap_entries - 1);
    multiboot_mmap_entry &e = entries[i];
    klog_debug("base_addr = %x", e.addr);
    klog_debug("length = %x", e.len);
  }
  // panic();
}

uint32_t multiboot_info::get_num_mmap_entries(multiboot_tag_mmap *mmap) {
  return (mmap->size - sizeof(multiboot_tag_mmap)) / mmap->entry_size;
}

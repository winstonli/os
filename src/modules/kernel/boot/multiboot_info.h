#pragma once

#include <cstdint>
#include <new>

#include <common/multiboot2.h>

#include <log.h>
#include <util/fixedsize_vector.h>

class PACKED multiboot_info {
 public:
  static multiboot_info *ptr;

  uint32_t total_size;
  uint32_t reserved;

  multiboot_tag_basic_meminfo *get_meminfo();
  multiboot_tag_mmap *get_mmap();

  static void init(void *multiboot_ptr);

  template<size_t VecSz>
  static void get_memory_map(
      multiboot_info *multiboot,
      fixedsize_vector<multiboot_tag_mmap, VecSz> &v
  ) {
    void *asdf;
    new (asdf) multiboot_info;
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
  }

  static uint32_t get_num_mmap_entries(multiboot_tag_mmap *mmap);
};

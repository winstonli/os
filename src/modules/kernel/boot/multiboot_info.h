#pragma once

#include <cstdint>
#include <new>

#include <common/multiboot2.h>

#include <assert.h>
#include <log.h>
#include <util/fixedsize_vector.h>

class PACKED multiboot_info {
 public:

  uint32_t total_size;
  uint32_t reserved;

  const multiboot_tag_basic_meminfo *get_meminfo() const;
  const multiboot_tag_mmap *get_mmap() const;

  template<size_t VecSz>
  void get_memory_map(
      fixedsize_vector<multiboot_mmap_entry, VecSz> &v
  ) const {
    klog_debug("Multiboot total_size = %x", total_size);
    klog_debug("Multiboot reserved = %x", reserved);
    const multiboot_tag_basic_meminfo *meminfo = get_meminfo();
    klog_debug("Meminfo type = %x", meminfo->type);
    klog_debug("Meminfo size = %x", meminfo->size);
    klog_debug("Meminfo mem_lower = %x", meminfo->mem_lower);
    klog_debug("Meminfo mem_upper = %x", meminfo->mem_upper);
    const multiboot_tag_mmap *mmap = get_mmap();
    klog_debug("Mmap type = %x", mmap->type);
    klog_debug("Mmap size = %x", mmap->size);
    klog_debug("Mmap entry_size = %x", mmap->entry_size);
    klog_debug("Mmap entry_version = %x", mmap->entry_version);
    uint32_t num_mmap_entries = get_num_mmap_entries(mmap);
    klog_debug("Num mmap entries: %x", num_mmap_entries);
    const multiboot_mmap_entry *entries = mmap->entries;
    for (uint32_t i = 0; i < num_mmap_entries; ++i) {
      klog_debug("Mmap entry %d/%d", i, num_mmap_entries - 1);
      const multiboot_mmap_entry &e = entries[i];
      klog_debug("base_addr = %x", e.addr);
      klog_debug("length = %x", e.len);
      v.push_back(e);
    }
    for (auto i = 0u; i < v.size(); ++i) {
      klog_debug("base in vec = %x", v[i].addr);
      klog_debug("length in vec = %x", v[i].len);
    }
  }

  static uint32_t get_num_mmap_entries(const multiboot_tag_mmap *mmap);
};

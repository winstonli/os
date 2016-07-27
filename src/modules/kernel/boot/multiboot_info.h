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
    const multiboot_tag_mmap *mmap = get_mmap();
    uint32_t num_mmap_entries = get_num_mmap_entries(mmap);
    const multiboot_mmap_entry *entries = mmap->entries;
    for (uint32_t i = 0; i < num_mmap_entries; ++i) {
      const multiboot_mmap_entry &e = entries[i];
      v.push_back(e);
    }
  }

  static uint32_t get_num_mmap_entries(const multiboot_tag_mmap *mmap);
};

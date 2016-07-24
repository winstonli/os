#pragma once

#include <stdint.h>

#include <common/multiboot2.h>

class PACKED multiboot_info {

public:
  static multiboot_info *ptr;

  uint32_t total_size;
  uint32_t reserved;

  multiboot_tag_basic_meminfo *get_meminfo();
  multiboot_tag_mmap *get_mmap();

  static void init(void *multiboot_ptr);

  static uint32_t get_num_mmap_entries(multiboot_tag_mmap *mmap);
};

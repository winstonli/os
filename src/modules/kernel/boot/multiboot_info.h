#pragma once

#include <stdint.h>

#include <common/multiboot2.h>

#include <vm/vm.h>

class PACKED multiboot_info {

public:
  static multiboot_info *ptr;

  uint32_t total_size;
  uint32_t reserved;

  multiboot_tag_basic_meminfo *get_meminfo();

  static void init(void *multiboot_ptr);
};

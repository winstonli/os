#pragma once

#include <stdint.h>

class vm {

  static constexpr uint64_t kernel_offset = 0xffff'fff8'0000'0000;

public:
  static void *paddr_to_kaddr(void *paddr);

  static void *kaddr_to_paddr(void *paddr);
};

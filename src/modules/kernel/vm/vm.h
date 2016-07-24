#pragma once

#include <stdint.h>

class vm {

  static constexpr uint64_t kernel_offset = 0xffff'ffff'8000'0000;

public:
  static void *paddr_to_kvaddr(void *paddr);

  static void *kvaddr_to_paddr(void *paddr);
};

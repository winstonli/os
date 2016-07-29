#include "msr.h"

uint64_t msr::read(uint32_t msr_id) {
  uint32_t low, high;
  asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr_id));
  return static_cast<uint64_t>(high) << 32 | low;
}

void msr::write(uint32_t msr_id, uint64_t new_value) {
  uint32_t low = static_cast<uint32_t>(new_value);
  uint32_t high = static_cast<uint32_t>(new_value >> 32);
  asm volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr_id));
}

#pragma once

#include <stdint.h>

struct cpuid_result_t {
  uint32_t eax;
  uint32_t edx;
};

// see http://wiki.osdev.org/CPUID
// performs the cpuid assembly function with the specified code in eax
cpuid_result_t cpuid(uint32_t code);

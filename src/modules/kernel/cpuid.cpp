#include "cpuid.h"

cpuid_result_t cpuid(uint32_t code) {
  cpuid_result_t res;
  asm volatile("cpuid"
               : "=a"(res.eax), "=d"(res.edx)
               : "a"(code)
               : "ecx", "ebx");
  return res;
}

#include "apic.h"
#include "cpuid.h"
#include "log.h"

#define CPUID_FLAG_APIC (1 << 9)

void apic::init() {
  klog_debug("Initialising Local APIC");
  auto cpuid_res = cpuid(1);
  klog_debug("Got CPUID result of a=%x d=%x", cpuid_res.eax, cpuid_res.edx);
  auto cpuid_supported = (cpuid_res.edx & CPUID_FLAG_APIC) != 0;
  klog_debug("Is APIC supported? %s", cpuid_supported ? "yes" : "no");
}

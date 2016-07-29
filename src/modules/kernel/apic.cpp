#include "apic.h"
#include <vm/vm.h>
#include "assert.h"
#include "cpuid.h"
#include "log.h"
#include "msr.h"

#define CPUID_FLAG_APIC (1 << 9)
#define APIC_BASE_MSR 0x1b
#define APIC_BASE_MSR_ENABLE 0x800

static void apic_set_base_paddr(uint64_t paddr) {
  assert(paddr == (paddr & 0xfffff100));
  msr::write(APIC_BASE_MSR, paddr | APIC_BASE_MSR_ENABLE);
}

static uint64_t apic_get_base_paddr() {
  auto addr = msr::read(APIC_BASE_MSR);
  return (addr & ~0xfffull);
}

void apic::init() {
  klog_debug("Initialising Local APIC");

  auto cpuid_res = cpuid(1);
  auto cpuid_apic_supported = (cpuid_res.edx & CPUID_FLAG_APIC) != 0;
  assertf(cpuid_apic_supported, "APIC not supported on this cpu!");

  auto base_paddr = apic_get_base_paddr();
  auto base_vaddr = vm::paddr_to_vaddr(reinterpret_cast<void*>(base_paddr));
  klog_debug("APIC base physical address is %x", base_paddr);
  klog_debug("APIC base virtual address is %x", base_vaddr);

  // hardware enable the apic if it wasn't already enabled by setting its
  // base to what it was already (usually 0xfee00000)
  apic_set_base_paddr(base_paddr);
}

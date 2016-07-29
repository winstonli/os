#include "apic.h"
#include <vm/vm.h>
#include "assert.h"
#include "cpuid.h"
#include "log.h"
#include "msr.h"

#define CPUID_FLAG_APIC (1 << 9)
#define APIC_BASE_MSR 0x1b
#define APIC_BASE_MSR_ENABLE 0x800

#define LAPIC_LVR_OFFSET 0x30
#define LAPIC_LVR_VERSION_MASK 0xff

static void apic_set_base_paddr(uint64_t paddr) {
  // paddr must be page aligned, cannot be past 4gb
  assert(paddr == (paddr & 0xfffff100));
  msr::write(APIC_BASE_MSR, paddr | APIC_BASE_MSR_ENABLE);
}

static uint64_t apic_get_base_paddr() {
  auto addr = msr::read(APIC_BASE_MSR);
  return (addr & ~0xfffull);
}

static uint64_t apic_get_base_vaddr() {
  auto base_paddr = apic_get_base_paddr();
  return reinterpret_cast<uint64_t>(
      vm::paddr_to_vaddr(reinterpret_cast<void*>(base_paddr)));
}

static uint32_t apic_read_register(uint64_t base_vaddr, uint32_t reg) {
  return *reinterpret_cast<volatile uint32_t*>(base_vaddr + reg);
}

static uint64_t apic_get_version(uint64_t base_vaddr) {
  auto data = apic_read_register(base_vaddr, LAPIC_LVR_OFFSET);
  return data & LAPIC_LVR_VERSION_MASK;
}

void apic::init() {
  klog_debug("Initialising Local APIC");

  auto cpuid_res = cpuid(1);
  auto cpuid_apic_supported = (cpuid_res.edx & CPUID_FLAG_APIC) != 0;
  assertf(cpuid_apic_supported, "APIC not supported on this cpu!");

  auto base_paddr = apic_get_base_paddr();
  auto base_vaddr = apic_get_base_vaddr();
  klog_debug("APIC base physical address is %x", base_paddr);
  klog_debug("APIC base virtual address is %x", base_vaddr);

  uint32_t version = apic_get_version(base_vaddr);
  klog_debug("APIC version is %x", version);

  // hardware enable the apic if it wasn't already enabled by setting its
  // base to what it was already (usually 0xfee00000)
  apic_set_base_paddr(base_paddr);
}

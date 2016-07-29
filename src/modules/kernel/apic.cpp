#include "apic.h"
#include <vm/vm.h>
#include "assert.h"
#include "cpuid.h"
#include "log.h"
#include "msr.h"

#define LAPIC_ID_REG 0x20       // local apic id
#define LAPIC_VERSION_REG 0x30  // local apic version
#define LAPIC_TASK_PRIORITY_REG 0x80
#define LAPIC_ARBITRATION_PRIORITY_REG 0x90
#define LAPIC_PROCESSOR_PRIORITY_REG 0xa0
#define LAPIC_END_OF_INTERRUPT_REG 0xb0
#define LAPIC_REMOTE_READ_REG 0xc0
#define LAPIC_LOGICAL_DESTINATION_REG 0xd0
#define LAPIC_DESTINATION_FORMAT_REG 0xe0
#define LAPIC_SPURIOUS_INTERRUPT_VECTOR_REG 0xf0
#define LAPIC_IN_SERVICE_REG_BASE 0x100         // x8 registers, +0x10 each
#define LAPIC_TRIGGER_MODE_REG_BASE 0x180       // x8
#define LAPIC_INTERRUPT_REQUEST_REG_BASE 0x200  // x8
#define LAPIC_ERROR_STATUS_REG 0x280
#define LAPIC_INTERRUPT_COMMAND_LOW_REG 0x300   // bits 0-31
#define LAPIC_INTERRUPT_COMMAND_HIGH_REG 0x310  // bits 32-63
#define LAPIC_TIMER_REG 0x320                   // lvt timer
#define LAPIC_THERMAL_REG 0x330                 // lvt thermal sensor
#define LAPIC_PERFORMANCE_COUNTER_REG 0x340     // lvt performance counter
#define LAPIC_LINT0_REG 0x350
#define LAPIC_LINT1_REG 0x360
#define LAPIC_ERROR_REG 0x370
#define LAPIC_TIMER_INITIAL_COUNT_REG 0x380
#define LAPIC_TIMER_CURRENT_COUNT_REG 0x390
#define LAPIC_TIMER_DIVIDE_CONFIG_REG 0x3e0

#define LAPIC_DESTINATION_FORMAT_FLAT 0xffffffffu

#define CPUID_FLAG_APIC (1 << 9)
#define APIC_BASE_MSR 0x1b
#define APIC_BASE_MSR_ENABLE 0x800

#define LAPIC_VERSION_MASK 0xff

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

static void apic_write_register(uint64_t base_vaddr, uint32_t reg,
                                uint32_t data) {
  *reinterpret_cast<volatile uint32_t*>(base_vaddr + reg) = data;
}

static uint8_t apic_get_version(uint64_t base_vaddr) {
  return apic_read_register(base_vaddr, LAPIC_VERSION_REG) & LAPIC_VERSION_MASK;
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

  // clear task priority to enable all interrupts
  apic_write_register(base_vaddr, LAPIC_TASK_PRIORITY_REG, 0);

  // set destination mode to flat, all cpus use logical id 1
  apic_write_register(base_vaddr, LAPIC_DESTINATION_FORMAT_REG, 0xffffffffu);
  apic_write_register(base_vaddr, LAPIC_LOGICAL_DESTINATION_REG, 0x01000000u);

  // setup spurious interrupt vector register
  uint32_t flags = 0x100 | 0xff;
  apic_write_register(base_vaddr, LAPIC_SPURIOUS_INTERRUPT_VECTOR_REG, flags);
}

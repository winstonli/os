#include "ioapic.h"
#include "log.h"

// select register then send/receive data
#define IOAPIC_REG 0x0
#define IOAPIC_DATA 0x10

// actual registers!
#define IOAPIC_ID 0x00
#define IOAPIC_VERSION 0x01
#define IOAPIC_ARBITRATION_ID 0x02
// each irq has two registers (e.g. 0x10/0x11, then 0x12/0x13 etc...)
// first register is low, second is high
#define IOAPIC_IRQ_BASE 0x10
#define IOAPIC_IRQ_DISABLE (1 << 16)

static uint32_t ioapic_read_register(uint64_t base_vaddr, uint32_t reg) {
  // select register, then read from data addr
  *reinterpret_cast<volatile uint32_t*>(base_vaddr + IOAPIC_REG) = reg;
  return *reinterpret_cast<volatile uint32_t*>(base_vaddr + IOAPIC_DATA);
}

static void ioapic_write_register(uint64_t base_vaddr, uint32_t reg,
                                  uint32_t data) {
  // select register, then write to data addr
  *reinterpret_cast<volatile uint32_t*>(base_vaddr + IOAPIC_REG) = reg;
  *reinterpret_cast<volatile uint32_t*>(base_vaddr + IOAPIC_DATA) = data;
}

static void ioapic_write_irq(uint64_t base_vaddr, uint8_t irq, uint64_t data) {
  auto low = static_cast<uint32_t>(data);
  auto high = static_cast<uint32_t>(data >> 32);
  ioapic_write_register(base_vaddr, IOAPIC_IRQ_BASE + irq * 2, low);
  ioapic_write_register(base_vaddr, IOAPIC_IRQ_BASE + irq * 2 + 1, high);
}

static uint32_t ioapic_get_version(uint64_t base_vaddr) {
  // stored in bits 0-8 inclusive
  return ioapic_read_register(base_vaddr, IOAPIC_VERSION) & 0x1ff;
}

static uint32_t ioapic_get_redirection_entries_count(uint64_t base_vaddr) {
  // stored in bits 16-23 inclusive as the maximum (so add 1 to get count)
  return ((ioapic_read_register(base_vaddr, IOAPIC_VERSION) >> 16) & 0xff) + 1;
}

void ioapic::init(acpi::config_t config) {
  auto base = config.ioapic_base_vaddr;
  if (!base) {
    klog_warn("Cannot initialise IO APIC: null base vaddr");
  }
  klog_debug("Initialising IO APIC with base vaddr %x", base);

  auto version = ioapic_get_version(base);
  auto redirection_entries = ioapic_get_redirection_entries_count(base);
  klog_debug("IOAPIC is version %x", version);
  klog_debug("IOAPIC has %d redirection entries", redirection_entries);

  for (uint8_t i = 0; i < redirection_entries; ++i) {
    ioapic_write_irq(base, i, IOAPIC_IRQ_DISABLE);
  }
}

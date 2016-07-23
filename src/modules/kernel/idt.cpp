#include "idt.h"
#include "common/common.h"
#include "common/string.h"
#include "terminal.h"

extern "C" void idt_flush(idt_ptr_t *idt_ptr);

STATIC idt_entry_t idt_entries[256];
STATIC idt_ptr_t idt_ptr;

void idt_set_handler(uint8_t num, void (*handler)()) {
  // these are constants until we actually need them to be arguments again
  uint16_t code_segment_selector = 0x08;
  uint8_t flags = 0x8e;
  uint64_t base = reinterpret_cast<uint64_t>(handler);

  idt_entries[num].base_low = base & 0xFFFF;
  idt_entries[num].base_mid = (base >> 16) & 0xFFFF;
  idt_entries[num].base_high = base >> 32;

  idt_entries[num].code_segment_selector = code_segment_selector;
  idt_entries[num].always_zero1 = 0;
  idt_entries[num].always_zero2 = 0;
  // TODO: We must uncomment the OR below when we get to using user-mode.
  // It sets the interrupt gate's privilege level to 3.
  idt_entries[num].flags = flags /* | 0x60 */;
}

void idt_init() {
  idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
  idt_ptr.base = (uint64_t)&idt_entries;

  memzero(&idt_entries[0], 256);

  idt_set_handler(0, &isr0);
  idt_set_handler(1, &isr1);
  idt_set_handler(2, &isr2);
  idt_set_handler(3, &isr3);
  idt_set_handler(4, &isr4);
  idt_set_handler(5, &isr5);
  idt_set_handler(6, &isr6);
  idt_set_handler(7, &isr7);
  idt_set_handler(8, &isr8);
  idt_set_handler(9, &isr9);
  idt_set_handler(10, &isr10);
  idt_set_handler(11, &isr11);
  idt_set_handler(12, &isr12);
  idt_set_handler(13, &isr13);
  idt_set_handler(14, &isr14);
  idt_set_handler(15, &isr15);
  idt_set_handler(16, &isr16);
  idt_set_handler(17, &isr17);
  idt_set_handler(18, &isr18);
  idt_set_handler(19, &isr19);
  idt_set_handler(20, &isr20);
  idt_set_handler(21, &isr21);
  idt_set_handler(22, &isr22);
  idt_set_handler(23, &isr23);
  idt_set_handler(24, &isr24);
  idt_set_handler(25, &isr25);
  idt_set_handler(26, &isr26);
  idt_set_handler(27, &isr27);
  idt_set_handler(28, &isr28);
  idt_set_handler(29, &isr29);
  idt_set_handler(30, &isr30);
  idt_set_handler(31, &isr31);

  // TODO: move this elsewhere
  idt_set_handler(0x20 + 0, &irq0);
  idt_set_handler(0x20 + 1, &irq1);
  idt_set_handler(0x20 + 2, &irq2);
  idt_set_handler(0x20 + 3, &irq3);
  idt_set_handler(0x20 + 4, &irq4);
  idt_set_handler(0x20 + 5, &irq5);
  idt_set_handler(0x20 + 6, &irq6);
  idt_set_handler(0x20 + 7, &irq7);
  idt_set_handler(0x20 + 8, &irq8);
  idt_set_handler(0x20 + 9, &irq9);
  idt_set_handler(0x20 + 10, &irq10);
  idt_set_handler(0x20 + 11, &irq11);
  idt_set_handler(0x20 + 12, &irq12);
  idt_set_handler(0x20 + 13, &irq13);
  idt_set_handler(0x20 + 14, &irq14);
  idt_set_handler(0x20 + 15, &irq15);

  idt_flush(&idt_ptr);
}

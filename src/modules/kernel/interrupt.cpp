#include "interrupt.h"
#include "common/common.h"
#include "common/string.h"
#include "terminal.h"

extern "C" void entry(void); // beginning of the module

extern "C" void idt_flush(uint64_t);

STATIC idt_entry_t idt_entries[256];
STATIC idt_ptr_t idt_ptr;

static void idt_set_gate(uint8_t num, uint64_t base,
                         uint16_t code_segment_selector, uint8_t flags) {
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

void init_interrupt_descriptor_table() {
  idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
  idt_ptr.base = (uint64_t)&idt_entries;

  memzero(&idt_entries[0], 256);

  // TODO: this is currently wrong because clang actually tries to dereference
  // the function address, printing out machine code instead of a pointer.
  auto isr0_addr = (uint64_t)&isr0;
  terminal_printf("isr0_addr is %x\n", isr0_addr);
  idt_set_gate(0, isr0_addr, 0x08, 0x8E);
  //  idt_set_gate(1, (uint64_t)isr1, 0x08, 0x8E);
  //  idt_set_gate(2, (uint64_t)isr2, 0x08, 0x8E);
  //  idt_set_gate(3, (uint64_t)isr3, 0x08, 0x8E);
  //  idt_set_gate(4, (uint64_t)isr4, 0x08, 0x8E);
  //  idt_set_gate(5, (uint64_t)isr5, 0x08, 0x8E);
  //  idt_set_gate(6, (uint64_t)isr6, 0x08, 0x8E);
  //  idt_set_gate(7, (uint64_t)isr7, 0x08, 0x8E);
  //  idt_set_gate(8, (uint64_t)isr8, 0x08, 0x8E);
  //  idt_set_gate(9, (uint64_t)isr9, 0x08, 0x8E);
  //  idt_set_gate(10, (uint64_t)isr10, 0x08, 0x8E);
  //  idt_set_gate(11, (uint64_t)isr11, 0x08, 0x8E);
  //  idt_set_gate(12, (uint64_t)isr12, 0x08, 0x8E);
  //  idt_set_gate(13, (uint64_t)isr13, 0x08, 0x8E);
  //  idt_set_gate(14, (uint64_t)isr14, 0x08, 0x8E);
  //  idt_set_gate(15, (uint64_t)isr15, 0x08, 0x8E);
  //  idt_set_gate(16, (uint64_t)isr16, 0x08, 0x8E);
  //  idt_set_gate(17, (uint64_t)isr17, 0x08, 0x8E);
  //  idt_set_gate(18, (uint64_t)isr18, 0x08, 0x8E);
  //  idt_set_gate(19, (uint64_t)isr19, 0x08, 0x8E);
  //  idt_set_gate(20, (uint64_t)isr20, 0x08, 0x8E);
  //  idt_set_gate(21, (uint64_t)isr21, 0x08, 0x8E);
  //  idt_set_gate(22, (uint64_t)isr22, 0x08, 0x8E);
  //  idt_set_gate(23, (uint64_t)isr23, 0x08, 0x8E);
  //  idt_set_gate(24, (uint64_t)isr24, 0x08, 0x8E);
  //  idt_set_gate(25, (uint64_t)isr25, 0x08, 0x8E);
  //  idt_set_gate(26, (uint64_t)isr26, 0x08, 0x8E);
  //  idt_set_gate(27, (uint64_t)isr27, 0x08, 0x8E);
  //  idt_set_gate(28, (uint64_t)isr28, 0x08, 0x8E);
  //  idt_set_gate(29, (uint64_t)isr29, 0x08, 0x8E);
  //  idt_set_gate(30, (uint64_t)isr30, 0x08, 0x8E);
  //  idt_set_gate(31, (uint64_t)isr31, 0x08, 0x8E);
  //
  //  idt_flush((uint64_t)&idt_ptr);
}

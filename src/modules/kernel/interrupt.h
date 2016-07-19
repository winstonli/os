#pragma once

#include "common/common.h"

// see JamesM's kernel development tutorials, section 4.4.
// see http://wiki.osdev.org/IDT#IDT_in_IA-32e_Mode_.2864-bit_IDT.29
// the terms "base" and "offset" are used interchangably in different tutorials,
// we stick to base here.

struct PACKED idt_entry_t {
  uint16_t base_low; // the lower 16 bits of the address to jump to when this
                     // interrupt fires.
  uint16_t code_segment_selector; // kernel code segment selector.
  uint8_t always_zero1;
  uint8_t flags;      // more flags. see documentation.
  uint16_t base_mid;  // the upper 16 bits of the address to jump to.
  uint32_t base_high; // the upper 16 bits of the address to jump to.
  uint32_t always_zero2;
};
STATIC_ASSERT(sizeof(idt_entry_t) == 16);

// a struct describing a pointer to an array of interrupt handlers.
// this is in a format suitable for giving to 'lidt'.
struct PACKED idt_ptr_t {
  uint16_t limit;
  uint64_t base; // the address of the first element in our idt_entry_t array.
};
STATIC_ASSERT(sizeof(idt_ptr_t) == 10);

// this must be called before interrupts have been enabled!
void init_interrupt_descriptor_table();

// these extern directives let us access the addresses of our asm isr
// handlers.
extern "C" void isr0();
extern "C" void isr1();
extern "C" void isr2();
extern "C" void isr3();
extern "C" void isr4();
extern "C" void isr5();
extern "C" void isr6();
extern "C" void isr7();
extern "C" void isr8();
extern "C" void isr9();
extern "C" void isr10();
extern "C" void isr11();
extern "C" void isr12();
extern "C" void isr13();
extern "C" void isr14();
extern "C" void isr15();
extern "C" void isr16();
extern "C" void isr17();
extern "C" void isr18();
extern "C" void isr19();
extern "C" void isr20();
extern "C" void isr21();
extern "C" void isr22();
extern "C" void isr23();
extern "C" void isr24();
extern "C" void isr25();
extern "C" void isr26();
extern "C" void isr27();
extern "C" void isr28();
extern "C" void isr29();
extern "C" void isr30();
extern "C" void isr31();

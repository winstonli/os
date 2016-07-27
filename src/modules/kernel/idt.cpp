#include "idt.h"
#include "common/common.h"
#include "common/string.h"
#include "terminal.h"

// see JamesM's kernel development tutorials, section 4.4.
// see http://wiki.osdev.org/IDT#IDT_in_IA-32e_Mode_.2864-bit_IDT.29
// the terms "base" and "offset" are used interchangably in different tutorials,
// we stick to base here.

struct PACKED idt_entry_t {
  uint16_t base_low;  // the lower 16 bits of the address to jump to when this
                      // interrupt fires.
  uint16_t code_segment_selector;  // kernel code segment selector.
  uint8_t always_zero1;
  uint8_t flags;       // more flags. see documentation.
  uint16_t base_mid;   // the upper 16 bits of the address to jump to.
  uint32_t base_high;  // the upper 16 bits of the address to jump to.
  uint32_t always_zero2;
};
STATIC_ASSERT(sizeof(void *) == sizeof(uint64_t));
STATIC_ASSERT(sizeof(idt_entry_t) == 16);

// a struct describing a pointer to an array of interrupt handlers.
// this is in a format suitable for giving to 'lidt'.
struct PACKED idt_ptr_t {
  uint16_t limit;
  uint64_t base;  // the address of the first element in our idt_entry_t array.
};
STATIC_ASSERT(sizeof(idt_ptr_t) == 10);

// set the idt_ptr_t used by the cpu
extern "C" void idt_flush(idt_ptr_t *idt_ptr);

#define IDT_NUM_ENTRIES 256

STATIC idt_entry_t idt_entries[IDT_NUM_ENTRIES];
STATIC idt_ptr_t idt_ptr;

void idt::set_handler(uint8_t num, void (*handler)()) {
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

void idt::init() {
  idt_ptr.limit = sizeof(idt_entry_t) * IDT_NUM_ENTRIES - 1;
  idt_ptr.base = (uint64_t)&idt_entries;

  memzero(&idt_entries[0], IDT_NUM_ENTRIES);

  idt_flush(&idt_ptr);
}

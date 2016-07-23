#include "idt.h"
#include "common/common.h"
#include "common/string.h"
#include "terminal.h"

extern "C" void idt_flush(idt_ptr_t *idt_ptr);

#define IDT_NUM_ENTRIES 256

STATIC idt_entry_t idt_entries[IDT_NUM_ENTRIES];
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
  idt_ptr.limit = sizeof(idt_entry_t) * IDT_NUM_ENTRIES - 1;
  idt_ptr.base = (uint64_t)&idt_entries;

  memzero(&idt_entries[0], IDT_NUM_ENTRIES);

  idt_flush(&idt_ptr);
}

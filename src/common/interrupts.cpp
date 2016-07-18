#include "interrupts.h"

void enable_interrupts(void) {
  asm volatile("sti");
}

void disable_interrupts(void) {
  asm volatile("cli");
}

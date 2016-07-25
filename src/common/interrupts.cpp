#include "interrupts.h"

void enable_interrupts() { asm volatile("sti"); }

void disable_interrupts() { asm volatile("cli"); }

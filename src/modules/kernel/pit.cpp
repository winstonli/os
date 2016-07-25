#include "pit.h"
#include "irq.h"
#include "registers.h"
#include "terminal.h"

#define PIT_IRQ 0

void pit_handler(const registers_t *regs UNUSED) { terminal_putchar('.'); }

void pit_init() { irq_register_handler(PIT_IRQ, &pit_handler); }

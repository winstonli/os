#include "pit.h"
#include "assert.h"
#include "common/string.h"
#include "irq.h"
#include "registers.h"
#include "terminal.h"

#define PIT_IRQ 0

#define MAX_HANDLERS 16
STATIC void (*handlers[MAX_HANDLERS])(const registers_t *);
STATIC uint8_t handler_count;

static void pit_handler(const registers_t *regs) {
  terminal_putchar('.');

  for (uint8_t i = 0; i < handler_count; ++i) {
    handlers[i](regs);
  }
}

void pit_init() {
  irq_register_handler(PIT_IRQ, &pit_handler);

  memzero(&handlers[0], MAX_HANDLERS);
  handler_count = 0;
}

void pit_register_periodic(void (*handler)(const registers_t *)) {
  assert(handler_count < MAX_HANDLERS);
  handlers[handler_count++] = handler;
}

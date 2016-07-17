#include "terminal.h"

// entry point of 64-bit kernel proper, as jumped to from entry.s
extern "C" void kernel_main(void) {
  terminal_init();

  terminal_push_cursor_state(79, 24, terminal_colour_t::WHITE,
                             terminal_colour_t::RED);
  terminal_putchar('.');
  terminal_pop_cursor_state();

  terminal_push_cursor_state(4, 10, terminal_colour_t::GREEN,
                             terminal_colour_t::BLACK);

  uint64_t rip = 0;
  asm volatile("leaq (%%rip), %0" : "=r"(rip));

  terminal_printf("Current instruction pointer is around: %x\n", rip);

  for (int i = -11; i < 11; ++i) {
    terminal_printf("%d,", i);
  }
  while (true) {
    for (int j = 0; j < 10; ++j) {
      for (int i = 0; i < 2000000; ++i) {
      }
      terminal_putchar('.');
    }
    terminal_putchar('X');
  }
}

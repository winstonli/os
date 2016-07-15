#include "terminal.h"

extern "C" void get_content(void) {
  terminal_init();

  terminal_push_cursor_state(79, 24, terminal_colour_t::WHITE,
                             terminal_colour_t::RED);
  terminal_putchar('.');
  terminal_pop_cursor_state();

  terminal_push_cursor_state(10, 10, terminal_colour_t::GREEN,
                             terminal_colour_t::BLACK);
  while (true) {
    for (int j = 0; j < 10; ++j) {
      for (int i = 0; i < 1000000; ++i) {
      }
      terminal_putchar('.');
    }
    terminal_putchar('X');
  }
}

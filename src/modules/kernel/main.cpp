#include "terminal.h"

extern "C" void get_content(void) {
  terminal_init();

  terminal_push_cursor_state(10, 10, terminal_colour_t::GREEN,
                             terminal_colour_t::BLACK);
  terminal_write("testing from C++ land");
}

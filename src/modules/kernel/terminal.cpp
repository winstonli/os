#include "terminal.h"
#include "common/common.h"
#include "common/stdarg.h"
#include "common/string.h"

#define TEXT_VIDEO_MEMORY ((volatile uint16_t *)(0xb8000))
#define TEXT_NUM_ROWS 25
#define TEXT_NUM_COLS 80
#define CURSOR_STATE_STACK_MAX_SIZE 16

struct terminal_cursor_state_t {
  uint8_t x, y, colour;
};

STATIC terminal_cursor_state_t cursor_state_stack[CURSOR_STATE_STACK_MAX_SIZE];
STATIC int cursor_state_stack_idx;

void terminal_init(void) {
  cursor_state_stack_idx = 0;
  memzero(&cursor_state_stack[0], CURSOR_STATE_STACK_MAX_SIZE);
  terminal_pop_cursor_state(); // initialise to default state
}

void terminal_push_cursor_state(uint8_t x, uint8_t y, terminal_colour_t fg,
                                terminal_colour_t bg) {
  // TODO: do something if the stack would overflow
  auto &elem = cursor_state_stack[++cursor_state_stack_idx];
  elem.x = x;
  elem.y = y;
  elem.colour = (static_cast<uint8_t>(bg) << 4) | static_cast<uint8_t>(fg);
}

void terminal_pop_cursor_state(void) {
  if (cursor_state_stack_idx == 0) {
    // cannot remove from empty stack, so restore default state
    auto &elem = cursor_state_stack[0];
    elem.x = elem.y = 0;
    elem.colour = 0x07; // white on black
  } else {
    --cursor_state_stack_idx;
  }
}

// shift the content of the terminal screen up by one line,
// does _NOT_ change the position of the cursor
void terminal_scrollback() {
  // shift everything up a row
  memcpy(TEXT_VIDEO_MEMORY, TEXT_VIDEO_MEMORY + TEXT_NUM_COLS,
         TEXT_NUM_COLS * (TEXT_NUM_ROWS - 1));
  // clear the last row
  memzero(TEXT_VIDEO_MEMORY + TEXT_NUM_COLS * (TEXT_NUM_ROWS - 1),
          TEXT_NUM_COLS);
}

void terminal_putchar(char ch) {
  auto &elem = cursor_state_stack[cursor_state_stack_idx];
  while (elem.y >= TEXT_NUM_ROWS) {
    terminal_scrollback();
    --elem.y;
  }
  auto where = TEXT_VIDEO_MEMORY + (elem.y * TEXT_NUM_COLS) + elem.x;
  switch (ch) {
  case '\n':
    elem.x = 0;
    ++elem.y;
    break;
  default:
    *where = ch | (elem.colour << 8);
    // move elem state ready for next character
    ++elem.x;
    if (elem.x >= TEXT_NUM_COLS) {
      ++elem.y;
      elem.x = 0;
    }
  }
}

void terminal_write(const char *str) {
  for (; *str != '\0'; ++str) {
    terminal_putchar(*str);
  }
}

void terminal_printf(const char *format, ...) {
  va_list params;
  va_start(params, format);

  for (; *format != '\0'; ++format) {
    terminal_putchar(*format);
  }

  va_end(params);
}

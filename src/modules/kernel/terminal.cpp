#include "terminal.h"
#include "common/common.h"
#include "common/string.h"

#include <stdarg.h>

#define TEXT_VIDEO_MEMORY                                                      \
  ((volatile uint16_t *)((int64_t)0xb8000 + 0xffff'ffff'8000'0000))
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

void terminal_set_colour(terminal_colour_t fg, terminal_colour_t bg) {
  auto &elem = cursor_state_stack[cursor_state_stack_idx];
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

void terminal_print_64bit_unsigned(uint64_t value, const char *alphabet) {
  bool seen_nonzero = false;
  for (int divisor = 60; divisor >= 0; divisor -= 4) {
    uint64_t digit_idx = (value >> divisor) & 0xf;
    if (digit_idx > 0 || seen_nonzero) {
      terminal_putchar(alphabet[digit_idx]);
      seen_nonzero = true;
    }
  }
  if (!seen_nonzero) {
    terminal_putchar('0');
  }
}

void terminal_print_hex(uint64_t value) {
  terminal_write("0x");
  bool seen_nonzero = false;
  for (int divisor = 60; divisor >= 0; divisor -= 4) {
    uint64_t digit_idx = (value >> divisor) & 0xf;
    if (digit_idx > 0 || seen_nonzero) {
      terminal_putchar("0123456789abcdef"[digit_idx]);
      seen_nonzero = true;
    }
  }
  if (!seen_nonzero) {
    terminal_putchar('0');
  }
}

void terminal_print_dec_unsigned(uint64_t value) {
  bool seen_nonzero = false;
  for (uint64_t divisor = 10000000000000000000ull; divisor > 0; divisor /= 10) {
    uint64_t digit_idx = (value / divisor) % 10;
    if (digit_idx > 0 || seen_nonzero) {
      terminal_putchar('0' + digit_idx);
      seen_nonzero = true;
    }
  }
  if (!seen_nonzero) {
    terminal_putchar('0');
  }
}

void terminal_print_dec_signed(int64_t value) {
  // TODO: we don't handle MIN_INT
  if (value < 0) {
    terminal_putchar('-');
    value = -value;
  }
  terminal_print_dec_unsigned(value);
}

void terminal_printf(const char *format, ...) {
  va_list params;
  va_start(params, format);

  bool got_percent = false;
  for (; *format != '\0'; ++format) {
    char ch = *format;
    if (ch == '%') {
      if (got_percent) {
        terminal_putchar('%');
      }
      got_percent = !got_percent;
      continue;
    }
    if (!got_percent) {
      terminal_putchar(ch);
      continue;
    }
    switch (ch) {
    case 's': {
      auto arg = va_arg(params, const char *);
      terminal_write(arg);
      got_percent = false;
      break;
    }
    case 'x': {
      auto arg = va_arg(params, uint64_t);
      terminal_print_hex(arg);
      got_percent = false;
      break;
    }
    case 'd': {
      auto arg = va_arg(params, int32_t);
      terminal_print_dec_signed(arg);
      got_percent = false;
      break;
    }
    default:
      terminal_putchar(ch);
      got_percent = false;
    }
  }

  va_end(params);
}

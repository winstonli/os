#include "terminal.h"
#include "common/common.h"
#include "common/string.h"

#include <stdarg.h>

#define TEXT_VIDEO_MEMORY \
  ((volatile uint16_t *)((int64_t)0xb8000 + 0xffff'ffff'8000'0000))
#define TEXT_NUM_ROWS 25
#define TEXT_NUM_COLS 80
#define CURSOR_STATE_STACK_MAX_SIZE 16

struct terminal_cursor_state_t {
  uint8_t x, y, colour;
};

STATIC terminal_cursor_state_t cursor_state_stack[CURSOR_STATE_STACK_MAX_SIZE];
STATIC int cursor_state_stack_idx;

void terminal::init() {
  cursor_state_stack_idx = 0;
  memzero(&cursor_state_stack[0], CURSOR_STATE_STACK_MAX_SIZE);
  terminal::pop_cursor_state();  // initialise to default state
}

void terminal::push_cursor_state(uint8_t x, uint8_t y, terminal::colour_t fg,
                                 terminal::colour_t bg) {
  // TODO: do something if the stack would overflow
  auto &elem = cursor_state_stack[++cursor_state_stack_idx];
  elem.x = x;
  elem.y = y;
  elem.colour = (static_cast<uint8_t>(bg) << 4) | static_cast<uint8_t>(fg);
}

void terminal::set_colour(terminal::colour_t fg, terminal::colour_t bg) {
  auto &elem = cursor_state_stack[cursor_state_stack_idx];
  elem.colour = (static_cast<uint8_t>(bg) << 4) | static_cast<uint8_t>(fg);
}

void terminal::pop_cursor_state(void) {
  if (cursor_state_stack_idx == 0) {
    // cannot remove from empty stack, so restore default state
    auto &elem = cursor_state_stack[0];
    elem.x = elem.y = 0;
    elem.colour = 0x07;  // white on black
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

void terminal::putchar(char ch) {
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

void terminal::write(const char *str) {
  for (; *str != '\0'; ++str) {
    terminal::putchar(*str);
  }
}

static void terminal_print_hex(uint64_t value) {
  terminal::write("0x");
  bool seen_nonzero = false;
  for (int divisor = 60; divisor >= 0; divisor -= 4) {
    uint64_t digit_idx = (value >> divisor) & 0xf;
    if (digit_idx > 0 || seen_nonzero) {
      terminal::putchar("0123456789abcdef"[digit_idx]);
      seen_nonzero = true;
    }
  }
  if (!seen_nonzero) {
    terminal::putchar('0');
  }
}

static void terminal_print_dec_unsigned(uint64_t value, int min_length UNUSED) {
  bool seen_nonzero = false;
  min_length = min_length <= 0 ? 1 : min_length;
  uint64_t min_length_cmp = 1;
  for (int i = 0; i < min_length; ++i) {
    min_length_cmp = min_length_cmp * 10;
  }
  for (uint64_t divisor = 10000000000000000000ull; divisor > 0; divisor /= 10) {
    uint64_t digit_idx = (value / divisor) % 10;
    if (digit_idx > 0 || seen_nonzero || divisor < min_length_cmp) {
      terminal::putchar('0' + digit_idx);
      seen_nonzero = true;
    }
  }
  if (!seen_nonzero) {
    terminal::putchar('0');
  }
}

static void terminal_print_dec_signed(int64_t value, int min_length) {
  // TODO: we don't handle MIN_INT
  if (value < 0) {
    terminal::putchar('-');
    value = -value;
  }
  terminal_print_dec_unsigned(value, min_length);
}

void terminal::printf(const char *format, ...) {
  va_list params;
  va_start(params, format);

  bool got_percent = false;
  bool got_zero = false;
  int zero_extend = 0;
  for (; *format != '\0'; ++format) {
    char ch = *format;
    if (ch == '%') {
      if (got_percent) {
        terminal::putchar('%');
      }
      got_percent = !got_percent;
      continue;
    }
    if (!got_percent) {
      terminal::putchar(ch);
      continue;
    }
    if (got_zero && ch >= '0' && ch <= '9') {
      zero_extend = zero_extend * 10 + (ch - '0');
      continue;
    }
    switch (ch) {
      case '0':
        got_zero = true;
        break;
      case 's': {
        auto arg = va_arg(params, const char *);
        terminal::write(arg);
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
        terminal_print_dec_signed(arg, zero_extend);
        got_percent = false;
        break;
      }
      default:
        terminal::putchar(ch);
        got_percent = false;
    }
    if (!got_percent) {
      zero_extend = false;
      got_zero = false;
    }
  }

  va_end(params);
}

void terminal::clear() {
  auto &elem = cursor_state_stack[cursor_state_stack_idx];
  elem.x = 0;
  elem.y = 0;
  memzero(TEXT_VIDEO_MEMORY, TEXT_NUM_ROWS * TEXT_NUM_COLS * 2);
}

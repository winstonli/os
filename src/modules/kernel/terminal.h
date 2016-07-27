#pragma once

#include <stdint.h>

namespace terminal {

enum class colour_t {
  BLACK = 0x0,
  BLUE = 0x1,
  GREEN = 0x2,
  CYAN = 0x3,
  RED = 0x4,
  MAGENTA = 0x5,
  BROWN = 0x6,
  LIGHT_GRAY = 0x7,
  DARK_GRAY = 0x8,
  LIGHT_BLUE = 0x9,
  LIGHT_GREEN = 0xa,
  LIGHT_CYAN = 0xb,
  LIGHT_RED = 0xc,
  LIGHT_MAGENTA = 0xd,
  YELLOW = 0xe,
  WHITE = 0xf
};

// this must be called before any of the below functions can be used
void init();

// sets the position of the cursor on the screen, and the colour for new text
// maintaining the old position in a stack
void push_cursor_state(uint8_t x, uint8_t y, colour_t fg, colour_t bg = colour_t::BLACK);

// just sets a colour
void set_colour(colour_t fg, colour_t bg = colour_t::BLACK);

// restore a previously held state of the cursor. if the stack is empty,
// restores to a default state
void pop_cursor_state(void);

// prints the specified character to the terminal
void putchar(char ch);

// prints the specified null-terminated string to the terminal
void write(const char *str);

// C-style printf to text console
void printf(const char *format, ...);

void clear();

}

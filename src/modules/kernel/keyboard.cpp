#include "keyboard.h"
#include "irq.h"
#include "registers.h"
#include "terminal.h"
#include "common/string.h"

#include <limits.h>

#define IRQ_PS2_KEYBOARD 1

#define NUM_SCANCODES (1 << CHAR_BIT)
STATIC char scancodes[NUM_SCANCODES];

static void keyboard_handler(const registers_t *regs UNUSED) {
  // we _must_ read from 0x60, even if we don't care about the result!
  auto scancode = in<uint8_t>(0x60);
  if (scancode & 0x80) {
    // do nothing for "key up" codes
  } else if (scancodes[scancode]) {
    terminal_putchar(scancodes[scancode]);
  } else {
    terminal_printf("(unknown scancode: %x)", scancode);
  }
}

void keyboard_init() {
  irq_register_handler(IRQ_PS2_KEYBOARD, &keyboard_handler);

  memzero(&scancodes[0], NUM_SCANCODES);

  scancodes[0x2] = '1';
  scancodes[0x3] = '2';
  scancodes[0x4] = '3';
  scancodes[0x5] = '4';
  scancodes[0x6] = '5';
  scancodes[0x7] = '6';
  scancodes[0x8] = '7';
  scancodes[0x9] = '8';
  scancodes[0xa] = '9';
  scancodes[0xb] = '0';
  scancodes[0xc] = '-';
  scancodes[0xd] = '=';

  scancodes[0x10] = 'q';
  scancodes[0x11] = 'w';
  scancodes[0x12] = 'e';
  scancodes[0x13] = 'r';
  scancodes[0x14] = 't';
  scancodes[0x15] = 'y';
  scancodes[0x16] = 'u';
  scancodes[0x17] = 'i';
  scancodes[0x18] = 'o';
  scancodes[0x19] = 'p';

  scancodes[0x1e] = 'a';
  scancodes[0x1f] = 's';
  scancodes[0x20] = 'd';
  scancodes[0x21] = 'f';
  scancodes[0x22] = 'g';
  scancodes[0x23] = 'h';
  scancodes[0x24] = 'j';
  scancodes[0x25] = 'k';
  scancodes[0x26] = 'l';
  scancodes[0x27] = ';';
  scancodes[0x28] = '\'';

  scancodes[0x2c] = 'z';
  scancodes[0x2d] = 'x';
  scancodes[0x2e] = 'c';
  scancodes[0x2f] = 'v';
  scancodes[0x30] = 'b';
  scancodes[0x31] = 'n';
  scancodes[0x32] = 'm';
  scancodes[0x33] = ',';
  scancodes[0x34] = '.';
  scancodes[0x35] = '/';

  scancodes[0x39] = ' ';
}

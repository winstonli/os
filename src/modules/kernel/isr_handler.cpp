#include "common/common.h"
#include "terminal.h"

extern "C" void isr_handler() {
  uint8_t x[1];
  terminal_printf("got an interrupt");
  for (int i = 65; i < 200; ++i) {
    if (x[i] < 0x20 && x[i] > 0)
      terminal_printf("stack index %d: %x\n", i, x[i]);
  }
}

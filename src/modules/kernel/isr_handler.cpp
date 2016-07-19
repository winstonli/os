#include "terminal.h"

extern "C" void isr_handler() {
  terminal_printf("got an interrupt!\n");
}

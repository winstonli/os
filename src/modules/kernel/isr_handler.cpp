#include "common/common.h"
#include "terminal.h"

STATIC int i = 0;

struct PACKED registers_t {
  uint64_t ds;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t rbp;
  uint64_t rbx;
  uint64_t rdx;
  uint64_t rcx;
  uint64_t rax;
  uint64_t int_no;
  uint64_t err_code;
};

struct PACKED gpf_err_code {
  uint16_t external : 1;
  uint16_t table : 2;
  uint16_t index : 13;
  uint16_t reserved;
};
STATIC_ASSERT(sizeof(gpf_err_code) == sizeof(uint32_t));

#define INT_GPF 0xd

extern "C" void isr_handler() {
  uint8_t x[1];
  terminal_printf("got an interrupt (%x with ip=%x)\n", x[105],
                  *(uint64_t *)&x[121]);
  for (int i = 65; i < 200; ++i) {
    if (x[i] < 0x20 && x[i] > 0) {
      //     terminal_printf("stack index %d: %x\n", i, x[i]);
    }
  }
}

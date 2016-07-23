#include "common/common.h"
#include "terminal.h"

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
  uint64_t rip;
};

struct PACKED gpf_err_code {
  uint16_t external : 1;
  uint16_t table : 2;
  uint16_t index : 13;
  uint16_t reserved;
};
STATIC_ASSERT(sizeof(gpf_err_code) == sizeof(uint32_t));

enum exception_code_t {
  EX_DIVIDE_BY_ZERO = 0x0,
  EX_DEBUG = 0x1,
  EX_NON_MASKABLE_INTERRUPT = 0x2,
  EX_BREAK = 0x3,
  EX_OVERFLOW = 0x4,
  EX_BOUND_RANGE_EXCEEDED = 0x5,
  EX_INVALID_OPCODE = 0x6,
  EX_DEVICE_NOT_AVAILABLE = 0x7,
  EX_DOUBLE_FAULT = 0x8,
  // EX_COPROCESSOR_SEGMENT_OVERRUN = 0x9 (now causes GPF)
  EX_INVALID_TSS = 0xa,
  EX_SEGMENT_NOT_PRESENT = 0xb,
  EX_STACK_SEGMENT_FAULT = 0xc,
  EX_GENERAL_PROTECTION_FAULT = 0xd,
  EX_PAGE_FAULT = 0xe,
  // RESERVED (0xf)
  EX_X87_FLOAT_EXCEPTION = 0x10,
  EX_ALIGNMENT_CHECK = 0x11,
  EX_MACHINE_CHECK = 0x12,
  EX_SIMD_FLOAT_CHECK = 0x13,
  EX_VIRTUALISATION_EXCEPTION = 0x14,
  // RESERVED (0x15 - 0x1d)
  EX_SECURITY_EXCEPTION = 0x1e,
  // RESERVED (0x1f)
};

extern "C" void isr_handler(const registers_t *regs) {
  auto gpf_err_ptr = reinterpret_cast<const gpf_err_code *>(&regs->err_code);
  switch (static_cast<exception_code_t>(regs->int_no)) {
  case EX_GENERAL_PROTECTION_FAULT:
    terminal_printf("\ngot a gpf at ip=%x with err_no=%x\n", regs->rip,
                    regs->err_code);
    terminal_printf("  (idx=%x, tbl=%x, external=%s)\n", gpf_err_ptr->index,
                    gpf_err_ptr->table, gpf_err_ptr->external ? "yes" : "no");
    break;
  default:
    terminal_printf("\ngot an interrupt\n  int_no=%x, err_no=%x, ip=%x)\n",
                    regs->int_no, regs->err_code, regs->rip);
  }
}

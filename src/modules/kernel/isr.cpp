#include "common/common.h"
#include "idt.h"
#include "terminal.h"

// these extern directives let us access the addresses of our asm isr
// handlers.
extern "C" void isr0();
extern "C" void isr1();
extern "C" void isr2();
extern "C" void isr3();
extern "C" void isr4();
extern "C" void isr5();
extern "C" void isr6();
extern "C" void isr7();
extern "C" void isr8();
extern "C" void isr9();
extern "C" void isr10();
extern "C" void isr11();
extern "C" void isr12();
extern "C" void isr13();
extern "C" void isr14();
extern "C" void isr15();
extern "C" void isr16();
extern "C" void isr17();
extern "C" void isr18();
extern "C" void isr19();
extern "C" void isr20();
extern "C" void isr21();
extern "C" void isr22();
extern "C" void isr23();
extern "C" void isr24();
extern "C" void isr25();
extern "C" void isr26();
extern "C" void isr27();
extern "C" void isr28();
extern "C" void isr29();
extern "C" void isr30();
extern "C" void isr31();

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

void isr_init() {
  // must be called after idt_init and before sti

  idt_set_handler(0, &isr0);
  idt_set_handler(1, &isr1);
  idt_set_handler(2, &isr2);
  idt_set_handler(3, &isr3);
  idt_set_handler(4, &isr4);
  idt_set_handler(5, &isr5);
  idt_set_handler(6, &isr6);
  idt_set_handler(7, &isr7);
  idt_set_handler(8, &isr8);
  idt_set_handler(9, &isr9);
  idt_set_handler(10, &isr10);
  idt_set_handler(11, &isr11);
  idt_set_handler(12, &isr12);
  idt_set_handler(13, &isr13);
  idt_set_handler(14, &isr14);
  idt_set_handler(15, &isr15);
  idt_set_handler(16, &isr16);
  idt_set_handler(17, &isr17);
  idt_set_handler(18, &isr18);
  idt_set_handler(19, &isr19);
  idt_set_handler(20, &isr20);
  idt_set_handler(21, &isr21);
  idt_set_handler(22, &isr22);
  idt_set_handler(23, &isr23);
  idt_set_handler(24, &isr24);
  idt_set_handler(25, &isr25);
  idt_set_handler(26, &isr26);
  idt_set_handler(27, &isr27);
  idt_set_handler(28, &isr28);
  idt_set_handler(29, &isr29);
  idt_set_handler(30, &isr30);
  idt_set_handler(31, &isr31);
}

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

#include "irq.h"
#include "idt.h"
#include "pic.h"
#include "registers.h"
#include "terminal.h"

// allows us to reference our asm irq handlers
extern "C" void irq0();
extern "C" void irq1();
extern "C" void irq2();
extern "C" void irq3();
extern "C" void irq4();
extern "C" void irq5();
extern "C" void irq6();
extern "C" void irq7();
extern "C" void irq8();
extern "C" void irq9();
extern "C" void irq10();
extern "C" void irq11();
extern "C" void irq12();
extern "C" void irq13();
extern "C" void irq14();
extern "C" void irq15();

void irq_init() {
  // must be called after idt_init and before sti
  // additionally, pic_init must be called before sti

  idt_set_handler(PIC1_OFFSET + 0, &irq0);
  idt_set_handler(PIC1_OFFSET + 1, &irq1);
  idt_set_handler(PIC1_OFFSET + 2, &irq2);
  idt_set_handler(PIC1_OFFSET + 3, &irq3);
  idt_set_handler(PIC1_OFFSET + 4, &irq4);
  idt_set_handler(PIC1_OFFSET + 5, &irq5);
  idt_set_handler(PIC1_OFFSET + 6, &irq6);
  idt_set_handler(PIC1_OFFSET + 7, &irq7);
  idt_set_handler(PIC1_OFFSET + 8, &irq8);
  idt_set_handler(PIC1_OFFSET + 9, &irq9);
  idt_set_handler(PIC1_OFFSET + 10, &irq10);
  idt_set_handler(PIC1_OFFSET + 11, &irq11);
  idt_set_handler(PIC1_OFFSET + 12, &irq12);
  idt_set_handler(PIC1_OFFSET + 13, &irq13);
  idt_set_handler(PIC1_OFFSET + 14, &irq14);
  idt_set_handler(PIC1_OFFSET + 15, &irq15);
}

extern "C" void irq_handler(const registers_t *regs) {
  terminal_printf("\ngot an interrupt request\n");
  terminal_printf("int_no=%x, err_no=%x, ip=%x)\n", regs->int_no,
                  regs->err_code, regs->rip);
}

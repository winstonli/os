#include "irq.h"
#include "assert.h"
#include "common/string.h"
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

#define HANDLERS_COUNT 16
STATIC void (*handlers[HANDLERS_COUNT])(const registers_t *);

void irq::init() {
  // must be called after idt_init and before sti
  // additionally, pic_init must be called before sti

  idt::set_handler(PIC1_OFFSET + 0, &irq0);
  idt::set_handler(PIC1_OFFSET + 1, &irq1);
  idt::set_handler(PIC1_OFFSET + 2, &irq2);
  idt::set_handler(PIC1_OFFSET + 3, &irq3);
  idt::set_handler(PIC1_OFFSET + 4, &irq4);
  idt::set_handler(PIC1_OFFSET + 5, &irq5);
  idt::set_handler(PIC1_OFFSET + 6, &irq6);
  idt::set_handler(PIC1_OFFSET + 7, &irq7);
  idt::set_handler(PIC1_OFFSET + 8, &irq8);
  idt::set_handler(PIC1_OFFSET + 9, &irq9);
  idt::set_handler(PIC1_OFFSET + 10, &irq10);
  idt::set_handler(PIC1_OFFSET + 11, &irq11);
  idt::set_handler(PIC1_OFFSET + 12, &irq12);
  idt::set_handler(PIC1_OFFSET + 13, &irq13);
  idt::set_handler(PIC1_OFFSET + 14, &irq14);
  idt::set_handler(PIC1_OFFSET + 15, &irq15);

  memzero(&handlers[0], HANDLERS_COUNT);
}

void irq::register_handler(uint8_t irq, void (*handler)(const registers_t *)) {
  assert(irq < HANDLERS_COUNT);
  handlers[irq] = handler;
}

extern "C" void irq_handler(const registers_t *regs) {
  auto irq_no = regs->int_no - PIC1_OFFSET;

  if (handlers[irq_no]) {
    handlers[irq_no](regs);
  }

  // tell the pic that we're ready for more interrupts!
  pic::send_eoi(irq_no);
}

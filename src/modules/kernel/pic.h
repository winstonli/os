#pragma once

#include <stdint.h>

// interrupts issued by the cpu itself (page faults etc...) occupy interrupts
// 0 through 31=0x1f, so we remap the pic (programmable interrupt controller)
// to issue us exceptions at 0x20 (anything out of the former range should be
// fine though)
#define PIC1_OFFSET 0x20  // new offset for master

// initialise the pic controller, must be called before interrupts are enabled
void pic_init();

// send an end-of-interrupt signal to the pic for the corresponding interrupt
// request number (where irq_no = int_no - PIC1_OFFSET)
// irq must be in the range 0 through 15 inclusive
void pic_send_eoi(uint8_t irq);

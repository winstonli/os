#pragma once

#include <stdint.h>

#define PIC1_OFFSET 0x20 // new offset for master (exceptions end at 0x1f)

void pic_init();

void pic_send_eoi(uint8_t irq);

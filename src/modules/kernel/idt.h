#pragma once

#include "common/common.h"

namespace idt {

// this must be called before interrupts have been enabled!
void init();

// registers the specified function as an interrupt handler for interrupt
// number 'num'
void set_handler(uint8_t num, void (*base)());

}

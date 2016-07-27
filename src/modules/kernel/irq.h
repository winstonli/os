#pragma once

#include "registers.h"

namespace irq {

// initialise handlers for software/externally-generated interrupts
// must be called after idt_init and before enabling interrupts
void init();

// registers the handler to be called whenever the specified irq is
// triggered.
void register_handler(uint8_t irq, void (*handler)(const registers_t*));

}

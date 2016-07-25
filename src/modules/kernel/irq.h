#pragma once

#include "registers.h"

// initialise handlers for software/externally-generated interrupts
// must be called after idt_init and before enabling interrupts
void irq_init();

// registers the handler to be called whenever the specified irq is
// triggered.
void irq_register_handler(uint8_t irq, void (*handler)(const registers_t*));

#pragma once

#include "registers.h"

// initialise the programmable interrupt timer
// must be called after irq_init
void pit_init();

// register a handler to be called each time a periodic interrupt occurs
// TODO: allow actually setting interrupt frequency
void pit_register_periodic(void(*handler)(const registers_t*));

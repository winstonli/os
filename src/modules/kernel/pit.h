#pragma once

#include "registers.h"

namespace pit {

// initialise the programmable interrupt timer
// must be called after irq_init
void init();

// register a handler to be called each time a periodic interrupt occurs
// TODO: allow actually setting interrupt frequency
void register_periodic(void(*handler)(const registers_t*));

}

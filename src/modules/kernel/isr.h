#pragma once

namespace isr {

// initialise handlers for cpu exceptions
// must be called after idt_init and before enabling interrupts
void init();

}

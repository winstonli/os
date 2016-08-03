#pragma once

#include "pic.h"

// to avoid clashes with the legacy pic, we position our interrupts immediately
// after it
#define LAPIC_OFFSET (PIC1_OFFSET+0x10)

namespace lapic {

// enable the local apic, disabling the 8259 pic in the progress (see pic.h)
void init();

}

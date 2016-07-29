#pragma once

namespace ioapic {

// enable the io apic, should be done after enabling local apic (see lapic.h)
void init();

}

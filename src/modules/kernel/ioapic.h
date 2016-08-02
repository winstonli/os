#pragma once

#include "acpi.h"

namespace ioapic {

// enable the io apic, should be done after enabling local apic (see lapic.h)
void init(acpi::config_t config);

}

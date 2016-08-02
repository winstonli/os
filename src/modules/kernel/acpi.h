#pragma once

#include <stdint.h>

namespace acpi {

struct config_t {
  uint64_t ioapic_base_vaddr; // TODO: what if there's more than one ioapic?
};

config_t init();

}

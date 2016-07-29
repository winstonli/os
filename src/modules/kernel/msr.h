#pragma once

#include <stdint.h>

namespace msr {

// see http://wiki.osdev.org/Model_Specific_Registers
// functions to read and modify the model specific registers

uint64_t read(uint32_t msr_id);

void write(uint32_t msr_id, uint64_t new_value);

}

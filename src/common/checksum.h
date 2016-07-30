#pragma once

#include <stdint.h>

// performs a checksum of the size bytes pointer to by data by adding up every
// byte in the structure
uint64_t checksum(const uint8_t *data, uint64_t size);

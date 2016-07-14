#pragma once

#include "stdint.h"

// write a piece of data of size T (uint{8,16,32}_t) to the specified port.
// specialisations provided in correspoding cpp file.
template<typename T>
void out(uint16_t port, T data);

// read a piece of data of size T (uint{8,16,32}_t) from the specified port.
// specialisations provided in correspoding cpp file.
template<typename T>
T in(uint16_t port);

#pragma once

#include "stdint.h"

// write a piece of data of size T (uint{8,16,32}_t) to the specified port.
// specialisations provided in correspoding cpp file.
template <typename T>
void out(uint16_t port, T data);

// read a piece of data of size T (uint{8,16,32}_t) from the specified port.
// specialisations provided in correspoding cpp file.
template <typename T>
T in(uint16_t port);

// forces the cpu to wait for an i/o operation to complete.
// should only be used if there is nothing to spin on to check for completion.
void io_wait();

// zero out the given array of elements
template <typename T>
void memzero(T *elems, int num_elems) {
  memzero(reinterpret_cast<char *>(elems), num_elems * sizeof(T));
}

// specialisation provided in corresponding cpp file.
template <>
void memzero(char *elems, int num_elems);

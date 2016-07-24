#pragma once

#include <stdint.h>

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

#define DATA [[gnu::section(".data")]]

// since we are in a land with no elf-loading we cannot correctly handle bss
// sections, so move any reserved space into .data to ensure we actually
// allocate it and mark it as static to ensure no lookup table entries are
// generated!
#define STATIC [[gnu::section(".data")]] static

// pack structures to avoid invisible padding
#define PACKED __attribute__((packed))

// static assert with no message
#define STATIC_ASSERT(cond) static_assert(cond, #cond)

#define NORETURN __attribute__((noreturn));

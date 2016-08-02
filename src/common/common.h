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

// since we are in a land with no elf-loading we cannot correctly handle bss
// sections, so move any reserved space into .data to ensure we actually
// allocate it and mark it as static to ensure no lookup table entries are
// generated!
#define DATA [[gnu::section(".data")]]
#define STATIC [[gnu::section(".data")]] static

// pack structures to avoid invisible padding
#define PACKED __attribute__((packed))

// static assert with no message
#define STATIC_ASSERT(cond) static_assert(cond, #cond)

// specify that a function does not return
#define NORETURN __attribute__((noreturn))

// for unused paramters
#define UNUSED __attribute__((unused))

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

static uint8_t operator""_u8(unsigned long long x) {
  return static_cast<uint8_t>(x);
}

static uint16_t operator""_u16(unsigned long long x) {
  return static_cast<uint16_t>(x);
}

static uint32_t operator""_u32(unsigned long long x) {
  return static_cast<uint32_t>(x);
}

static uint64_t operator""_u64(unsigned long long x) {
  return static_cast<uint64_t>(x);
}

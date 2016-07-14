#include "common.h"

template <>
void out(uint16_t port, uint8_t data) {
  asm volatile("outb %1, %0" : : "dN"(port), "a"(data));
}

template <>
void out(uint16_t port, uint16_t data) {
  asm volatile("outw %1, %0" : : "dN"(port), "a"(data));
}

template <>
void out(uint16_t port, uint32_t data) {
  asm volatile("outl %1, %0" : : "dN"(port), "a"(data));
}

template <>
uint8_t in(uint16_t port) {
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
  return ret;
}

template <>
uint16_t in(uint16_t port) {
  uint16_t ret;
  asm volatile("inw %1, %0" : "=a"(ret) : "dN"(port));
  return ret;
}

template <>
uint32_t in(uint16_t port) {
  uint32_t ret;
  asm volatile("inl %1, %0" : "=a"(ret) : "dN"(port));
  return ret;
}

void io_wait() {
  // Port 0x80 is used for 'checkpoints' during POST.
  // The Linux kernel seems to think it is free for use.
  out<uint8_t>(0x80, 0);
}

template <>
void memzero(char *elems, int num_elems) {
  for (int i = 0; i < num_elems; ++i) {
    elems[i] = 0;
  }
}

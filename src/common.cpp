#include "common.h"

template<>
void out(uint16_t port, uint8_t data) {
  asm volatile ("outb %1, %0" : : "dN" (port), "a" (data));
}

template<>
void out(uint16_t port, uint16_t data) {
  asm volatile ("outw %1, %0" : : "dN" (port), "a" (data));
}

template<>
void out(uint16_t port, uint32_t data) {
  asm volatile ("outl %1, %0" : : "dN" (port), "a" (data));
}

template<>
uint8_t in(uint16_t port) {
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}

template<>
uint16_t in(uint16_t port) {
  uint16_t ret;
  asm volatile("inw %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}

template<>
uint32_t in(uint16_t port) {
  uint32_t ret;
  asm volatile("inl %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}

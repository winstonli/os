#include <string.h>
#include "common.h"

extern "C" int memcmp(const void *p1, const void *p2, size_t n) {
  auto s1 = reinterpret_cast<const char *>(p1);
  auto s2 = reinterpret_cast<const char *>(p2);
  for (; n > 0 && *s1 == *s2; --n, ++s1, ++s2)
    ;
  return n == 0 ? 0 : *s1 - *s2;
}

extern "C" int strcmp(const char *s1, const char *s2) {
  for (; *s1 == *s2 && *s1 != '\0'; ++s1, ++s2)
    ;
  return *s1 - *s2;
}

extern "C" int strncmp(const char *s1, const char *s2, size_t n) {
  for (; n > 0 && *s1 == *s2 && *s1 != '\0'; --n, ++s1, ++s2)
    ;
  return n == 0 ? 0 : *s1 - *s2;
}

extern "C" void *memset(void *b, int c, size_t len) {
  char *dest = static_cast<char *>(b);
  for (auto i = 0u; i < len; ++i) {
    dest[i] = static_cast<unsigned char>(c);
  }
  return b;
}

extern "C" void memcpy_volatile(volatile void *d, volatile void *s,
                                uint64_t n) {
  auto dst = reinterpret_cast<volatile char *>(d);
  auto src = reinterpret_cast<volatile char *>(s);
  if (dst < src) {
    // if dst < src then we want to iterate forwards through src to avoid
    // overwriting elements in overlapping ranges
    for (decltype(n) i = 0; i < n; ++i) {
      *(dst++) = *(src++);
    }
  } else {
    // if dst > src then we want to iterate backwards for the same reason
    dst += n;
    src += n;
    for (decltype(n) i = 0; i < n; ++i) {
      *(--dst) = *(--src);
    }
  }
}

extern "C" void memset_volatile(volatile void *s, char c, uint64_t n) {
  auto data = reinterpret_cast<volatile char *>(s);
  for (decltype(n) i = 0; i < n; ++i) {
    *(data++) = c;
  }
}

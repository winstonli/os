#include "string.h"

template <>
void memzero(char *elems, int num_elems) {
  for (int i = 0; i < num_elems; ++i) {
    elems[i] = 0;
  }
}

template <>
void memzero(volatile char *elems, int num_elems) {
  for (int i = 0; i < num_elems; ++i) {
    elems[i] = 0;
  }
}

template <>
void memcpy(char *dst, char *src, int num_elems) {
  if (dst < src) {
    // if dst < src then we want to iterate forwards through src to avoid
    // overwriting elements in overlapping ranges
    for (int i = 0; i < num_elems; ++i) {
      dst[i] = src[i];
    }
  } else {
    // if dst > src then we want to iterate backwards for the same reason
    for (int i = num_elems - 1; i >= 0; --i) {
      dst[i] = src[i];
    }
  }
}

template <>
void memcpy(volatile char *dst, volatile char *src, int num_elems) {
  if (dst < src) {
    // if dst < src then we want to iterate forwards through src to avoid
    // overwriting elements in overlapping ranges
    for (int i = 0; i < num_elems; ++i) {
      dst[i] = src[i];
    }
  } else {
    // if dst > src then we want to iterate backwards for the same reason
    for (int i = num_elems - 1; i >= 0; --i) {
      dst[i] = src[i];
    }
  }
}

extern "C" {

int memcmp(const char *s1, const char *s2, size_t n) {
  for (; n > 0 && *s1 == *s2; --n, ++s1, ++s2)
    ;
  return n == 0 ? 0 : *s1 - *s2;
}

int strcmp(const char *s1, const char *s2) {
  for (; *s1 == *s2 && *s1 != '\0'; ++s1, ++s2)
    ;
  return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for (; n > 0 && *s1 == *s2 && *s1 != '\0'; --n, ++s1, ++s2)
    ;
  return n == 0 ? 0 : *s1 - *s2;
}

void *memset(void *b, int c, size_t len) {
  char *dest = static_cast<char *>(b);
  for (auto i = 0u; i < len; ++i) {
    dest[i] = static_cast<unsigned char>(c);
  }
  return b;
}
}

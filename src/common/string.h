#pragma once

#include <type_traits>

#include <stddef.h>

// zero out the given array of elements
template <typename T>
auto memzero(T *elems, int num_elems) -> std::enable_if_t<!std::is_volatile<T>::value> {
  memzero(reinterpret_cast<char *>(elems), num_elems * sizeof(T));
}

template <typename T>
auto memzero(T *elems, int num_elems) -> std::enable_if_t<std::is_volatile<T>::value> {
  memzero(reinterpret_cast<volatile char *>(elems), num_elems * sizeof(T));
}

// specialisations provided in corresponding cpp file.
template <>
void memzero(char *elems, int num_elems);

template <typename T>
auto memcpy(T *dst, T *src, int num_elems) -> std::enable_if_t<!std::is_volatile<T>::value> {
  memcpy(reinterpret_cast<char *>(dst), reinterpret_cast<char *>(src),
         num_elems * sizeof(T));
}

template <typename T>
auto memcpy(T *dst, T *src, int num_elems) -> std::enable_if_t<std::is_volatile<T>::value> {
  memcpy(reinterpret_cast<volatile char *>(dst),
         reinterpret_cast<volatile char *>(src), num_elems * sizeof(T));
}

// specialisations provided in corresponding cpp file.
template <>
void memcpy(char *dst, char *src, int num_elems);

template <>
void memcpy(volatile char *dst, volatile char *src, int num_elems);

/* Functions required by the C++ compiler */

extern "C" {

void *memset(void *b, int c, size_t len);

}

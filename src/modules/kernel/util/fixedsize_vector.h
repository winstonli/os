#pragma once

#include <array>
#include <utility>

template<typename T, std::size_t N>
class fixedsize_vector {

  std::array<T, N> arr;
  std::size_t _size;

public:

  T &operator[](size_t i) { return arr[i]; }

  const T &operator[](size_t i) const { return arr[i]; }

  size_t size() { return _size; }

  const T *data() const { return arr.data(); }

  T *data() { return arr.data(); }

  void push_back(const T &val) {
    arr[_size++] = val;
  }

  void push_back(T &&val) {
    arr[_size++] = std::move(val);
  }

};

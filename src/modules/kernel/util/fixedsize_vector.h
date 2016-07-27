#pragma once

#include <array>
#include <new>
#include <utility>

template<typename T, std::size_t N>
class fixedsize_vector {

  std::array<char, sizeof(T) * N> arr;
  std::size_t _size;

public:

  fixedsize_vector() : arr(), _size(0) {}

  fixedsize_vector(
      const fixedsize_vector &other
  ) : arr(),
      _size(other._size) {
    copy_elems(other);
  }

  fixedsize_vector &operator=(const fixedsize_vector &other) {
    _size = other._size;
    copy_elems(other);
  }

  fixedsize_vector(
      fixedsize_vector &&other
  ) : arr(),
      _size(other._size) {
    other._size = 0;
    move_elems(other);
  }

  fixedsize_vector &operator=(fixedsize_vector &&other) {
    std::swap(_size, other._size);
    move_elems(other);
  }

  ~fixedsize_vector() {
    T *elems = data();
    for (auto i = 0u; i < _size; ++i) {
      elems[i].~T();
    }
  }

  T &operator[](size_t i) { return data()[i]; }

  const T &operator[](size_t i) const { return data()[i]; }

  size_t size() const { return _size; }

  const T *data() const { return reinterpret_cast<const T *>(arr.data()); }

  T *data() { return reinterpret_cast<T *>(arr.data()); }

  /* 
     We don't use universal refs here because we want to be specific, and don't
     want these functions overloading on any old T.

     We use placement new to construct directly into the destination memory.
   */

  void push_back(const T &val) {
    new (data() + _size++) T(val);
  }

  void push_back(T &&val) {
    new (data() + _size++) T(std::move(val));
  }

  template <class... _Args>
  void emplace_back(_Args&&... __args) {
    new (data() + _size++) T(std::forward<_Args>(__args)...);
  }

  void pop_back() {
    data()[_size--].~T();
  }

private:

  void copy_elems(const fixedsize_vector &other) {
    T *d = data();
    const T *other_d = other.data();
    for (auto i = 0; i < _size; ++i) {
      d[i] = other_d[i];
    }
  }

  void move_elems(fixedsize_vector &&other) {
    T *d = data();
    const T *other_d = other.data();
    for (auto i = 0; i < _size; ++i) {
      std::swap(d[i], other_d[i]);
    }
  }

};

#pragma once

#include <util/emb_list_impl.h>

/* 
   Your emb_list_entry member must be called default_entry to use this class.
 */
template<typename T>
class emb_list {

public:

  struct iterator {

    emb_list_impl::iterator inner;

    iterator(emb_list_impl::iterator i) : inner(i) {}
    iterator(T &t) : inner(&t.default_entry) {}

    operator emb_list_impl::iterator &() {
      return inner;
    }

    operator emb_list_impl::iterator() const {
      return inner;
    }

    bool operator==(const iterator &other) const {
      return inner.operator==(other.inner);
    }

    bool operator!=(const iterator &other) const {
      return inner.operator!=(other.inner);
    }

    T &operator*() const {
      return inner.operator*();
    }

    T *operator->() const {
      return list_entry(inner.operator->(), T, default_entry);
    }

    iterator &operator++() {
      inner.operator++();
      return *this;
    }

    iterator operator++(int dummy) {
      return inner.operator++(dummy);
    }

  };

  struct const_iterator {

    emb_list_impl::const_iterator inner;

    const_iterator(emb_list_impl::const_iterator i) : inner(i) {}

    const_iterator(T &t) : inner(&t.default_entry) {}

    operator emb_list_impl::const_iterator &() {
      return inner;
    }

    operator emb_list_impl::const_iterator() const {
      return inner;
    }

    bool operator==(const const_iterator &other) const {
      return inner.operator==(other.inner);
    }

    bool operator!=(const const_iterator &other) const {
      return inner.operator!=(other.inner);
    }

    T &operator*() const {
      return inner.operator*();
    }

    T *operator->() const {
      return list_entry(inner.operator->(), T, default_entry);
    }

    const_iterator &operator++() {
      return inner.operator++();
    }

    const_iterator operator++(int dummy) {
      return inner.operator++(dummy);
    }

  };

private:

  emb_list_impl inner;

public:

  iterator begin() {
    return inner.begin();
  }

  const_iterator begin() const {
    return inner.begin();
  }

  iterator end() {
    return inner.end();
  }

  const_iterator end() const {
    return inner.end();
  }

  bool empty() {
    return inner.empty();
  }

  iterator erase(iterator pos) {
    return inner.erase(pos);
  }

  void push_front(T &t) {
    inner.push_front(t.default_entry);
  }

  void push_back(T &t) {
    inner.push_back(t.default_entry);
  }

};

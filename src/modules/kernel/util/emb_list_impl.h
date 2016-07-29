#pragma once

#include <cstddef>

#include <util/emb_list_entry.h>

#define list_entry(ptr, type, member) \
((type *)(uintptr_t)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

class emb_list_impl {

public:

  class iterator {

    emb_list_entry *current;

  public:

    iterator(emb_list_entry *first) : current(first) {}
    iterator(const iterator &) = default;

    bool operator==(const iterator &other) const {
      return current == other.current;
    }

    bool operator!=(const iterator &other) const {
      return !(*this == other);
    }

    emb_list_entry &operator*() const {
      return *current;
    }

    emb_list_entry *operator->() const {
      return current;
    }

    iterator &operator++() {
      current = current->next;
      return *this;
    }

    iterator operator++(int) {
      iterator ret(*this);
      ++(*this);
      return ret;
    }

  };

  class const_iterator {

    const emb_list_entry *current;

  public:

    const_iterator(const emb_list_entry *first) : current(first) {}

    bool operator==(const const_iterator &other) const {
      return current == other.current;
    }

    bool operator!=(const const_iterator &other) const {
      return !(*this == other);
    }

    const emb_list_entry &operator*() const {
      return *current;
    }

    const emb_list_entry *operator->() const {
      return current;
    }

    const_iterator &operator++() {
      current = current->next;
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator ret(*this);
      ++(*this);
      return ret;
    }

  };

private:

  emb_list_entry *head;
  emb_list_entry *last;

public:

  emb_list_impl();
  emb_list_impl(const emb_list_impl &) = delete;
  emb_list_impl &operator=(const emb_list_impl &) = delete;
  emb_list_impl(emb_list_impl &&) = delete;
  emb_list_impl &operator=(emb_list_impl &&) = delete;
  ~emb_list_impl() = default;

  iterator begin();
  const_iterator begin() const;

  iterator end();
  const_iterator end() const;

  bool empty();

  iterator erase(iterator pos);

  void push_front(emb_list_entry &e);

  void push_back(emb_list_entry &e);

};

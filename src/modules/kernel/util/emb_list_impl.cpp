#include "emb_list_impl.h"

emb_list_impl::emb_list_impl() : head(nullptr), last(nullptr) {}

emb_list_impl::iterator emb_list_impl::begin() { return head; }

emb_list_impl::const_iterator emb_list_impl::begin() const { return head; }

emb_list_impl::iterator emb_list_impl::end() { return nullptr; }

emb_list_impl::const_iterator emb_list_impl::end() const { return nullptr; }

bool emb_list_impl::empty() { return begin() == end(); }

emb_list_impl::iterator emb_list_impl::erase(emb_list_impl::iterator pos) {
  emb_list_entry *prev = pos->prev;
  emb_list_entry *next = pos->next;
  if (prev != nullptr) {
    prev->next = next;
  }
  if (next != nullptr) {
    next->prev = prev;
  }
  if (pos == head) {
    head = next;
  }
  if (pos == last) {
    last = prev;
  }
  return next;
}

void emb_list_impl::push_front(emb_list_entry &e) {
  emb_list_entry *next = head;
  head = &e;
  if (last == nullptr) {
    last = &e;
  }
  if (next != nullptr) {
    next->prev = &e;
  }
  e.prev = nullptr;
  e.next = next;
}

void emb_list_impl::push_back(emb_list_entry &e) {
  emb_list_entry *prev = last;
  last = &e;
  if (head == nullptr) {
    head = &e;
  }
  if (prev != nullptr) {
    prev->next = &e;
  }
  e.prev = prev;
  e.next = nullptr;
}

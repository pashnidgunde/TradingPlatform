#pragma once

#include "LinkedList.h"
#include <cstddef>

namespace platform {

template <typename T> struct Node {
  Node *prev = nullptr;
  T value{};
  Node *next = nullptr;
  T &get() { return value; }
  const T &get() const { return value; }
};

template <typename T, size_t SIZE> struct NodePool {
  NodePool() {
    head = new Node<T>();
    auto prev = head;
    for (size_t i = 0; i < SIZE - 1; ++i) {
      auto nn = new Node<T>();
      prev->next = nn;
      prev = nn;
    }
    tail = prev;
  }

  Node<T> *get() {
    if (!head) {
      head = new Node<T>();
      return head;
    }
    auto node = head;
    head = head->next;
    return node;
  }

  void put(Node<T> *node) {
    if (tail) {
      tail->next = node;
      tail = tail->next;
    } else {
      head = tail;
    }
  }

  Node<T> *head = nullptr;
  Node<T> *tail = nullptr;
};
} // namespace platform
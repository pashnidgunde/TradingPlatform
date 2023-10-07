#pragma once

#include <algorithm>
#include <iostream>

namespace platform {

template <typename T> struct Node {
  Node *prev = nullptr;
  T value{};
  Node *next = nullptr;

  explicit Node(T v) : value(std::move(v)) {}

  T& get() { return value; }
  const T& get() const { return value; }
};

template <typename T, typename Comp> struct LinkedList {
public:
  using NodePtr = Node<T> *;
  using value_type = NodePtr;

  struct IteratorBase {
    IteratorBase() = delete;
    explicit IteratorBase(NodePtr other) { node = other; }
    IteratorBase(const IteratorBase &other) { node = other.node; }
    IteratorBase &operator=(const IteratorBase &other) {
      node = other.node;
      return *this;
    }
    NodePtr operator*() const { return node; }
    NodePtr operator->() const { return node; }
    explicit operator NodePtr() const { return node; }
    explicit operator bool () const { return node != nullptr; }
    bool operator== (const IteratorBase& rhs) const {
      return this->node == rhs.node;
    }
    bool operator!= (const IteratorBase& rhs) const {
      return this->node != rhs.node;
    }

    NodePtr node = nullptr;
  };

  class Iterator : public IteratorBase {
  public:
    explicit Iterator(NodePtr other) : IteratorBase(other)
    {}
    Iterator &operator++() {
      this->node = this->node->next;
      return *this;
    }
    const Iterator operator++(int) {
      Iterator old = *this;
      this->node = this->node->next;
      return old;
    }
  };

  class ReverseIterator : public IteratorBase {
  public:
    explicit ReverseIterator(NodePtr other) : IteratorBase(other) {}
    ReverseIterator &operator++() {
      this->node = this->node->prev;
      return *this;
    }
    const ReverseIterator operator++(int) {
      ReverseIterator old = *this;
      this->node = this->node->prev;
      return old;
    }
  };

  bool isEmpty() const { return head == nullptr; }

  [[nodiscard]] NodePtr insert(const T &data) {
    auto node = new Node<T>(data);
    if (!head) {
      head = node;
      tail = head;
      sz = 1;
    } else {
      this->comparedInsert(node);
      sz++;
    }
    return node;
  }

  void removeIf(const std::function<bool(NodePtr node)> fn) {
      NodePtr prev = nullptr;
      while (head && fn(head)) {
          prev = head;
          head = head->next;
          delete prev;

          sz--;
      }
  }

  size_t size() const { return sz; }
  Iterator begin() { return Iterator(head); };
  Iterator end() { return Iterator(nullptr); }
  ReverseIterator rbegin() { return ReverseIterator(tail); }
  ReverseIterator rend() { return ReverseIterator(nullptr); }

private:
  void comparedInsert(NodePtr nn) {
    // should insert at start ?
    if (compare(nn->value, head->value)) {
      nn->next = head;
      head->prev = nn;
      head = nn;
      return;
    }

    // should insert at end ?
    if (compare(tail->value, nn->value) || tail->value == nn->value) {
      tail->next = nn;
      nn->prev = tail;
      tail = nn;
      return;
    }

    auto it = begin();
    while (it) {
      if (compare(nn->value, it->value)) {
        nn->next = *it;
        nn->prev = it->prev;
        nn->prev->next = nn;
        it->prev = nn;
        return;
      }
      it++;
    }
  }

private:
  NodePtr head = nullptr;
  NodePtr tail = nullptr;
  size_t sz = 0;
  Comp compare{};
};

} // namespace platform

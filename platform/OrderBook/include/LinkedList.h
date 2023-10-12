#pragma once

#include <algorithm>
#include <functional>
#include <iostream>

#include "NodePool.h"

namespace platform {

    template<typename T>
    struct LinkedList {
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

            explicit operator bool() const { return node != nullptr; }

            bool operator==(const IteratorBase &rhs) const {
                return this->node == rhs.node;
            }

            bool operator!=(const IteratorBase &rhs) const {
                return this->node != rhs.node;
            }

            NodePtr node = nullptr;
        };

        class Iterator : public IteratorBase {
        public:
            explicit Iterator(NodePtr other) : IteratorBase(other) {}

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

        [[nodiscard]] bool isEmpty() const { return head == nullptr; }

        [[nodiscard]] NodePtr insert(const T &data) {
            auto node = pool.get();
            node->value = data;
            if (!head) {
                head = node;
                tail = head;
                sz = 1;
            } else {
                tail->next = node;
                node->prev = tail;
                tail = node;
                sz++;
            }
            return node;
        }

        void removeIf(const std::function<bool(NodePtr node)> fn) {
            while (head && fn(head)) {
                NodePtr prev = head;
                head = head->next;
                pool.put(prev);
                sz--;
            }
        }

        [[nodiscard]] size_t size() const { return sz; }

        Iterator begin() { return Iterator(head); };

        Iterator end() { return Iterator(nullptr); }

        ReverseIterator rbegin() { return ReverseIterator(tail); }

        ReverseIterator rend() { return ReverseIterator(nullptr); }

        const T &front() const { return head->get(); }

    private:
        NodePool<T, 1024> pool;
        NodePtr head = nullptr;
        NodePtr tail = nullptr;
        size_t sz = 0;
    };

}  // namespace platform


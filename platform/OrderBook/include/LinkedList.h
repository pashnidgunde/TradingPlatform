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

            Iterator operator++(int) {
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

            ReverseIterator operator++(int) {
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

        [[nodiscard]] NodePtr insert(T &&data) {
            auto node = pool.get();
            node->value = std::move(data);
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

        void clear() {
            this->removeIf([](auto node) { return node != nullptr; });
            head = tail = nullptr;
            sz = 0;
        }

        void remove(NodePtr node) {
            if (!node) return;
            if (node->prev) {
                node->prev->next = node->next;
            }
            if (node->next) {
                node->next->prev = node->prev;
            }
            sz--;
            pool.put(node);
        }

        [[nodiscard]] size_t size() const { return sz; }

        Iterator begin() const { return Iterator(head); };

        Iterator end() const { return Iterator(nullptr); }

        ReverseIterator rbegin() const { return ReverseIterator(tail); }

        ReverseIterator rend() const { return ReverseIterator(nullptr); }

        const T &front() const { return head->get(); }

        NodePtr frontNode() const { return head; }

    private:
        NodePool<T, 1024> pool;
        NodePtr head = nullptr;
        NodePtr tail = nullptr;
        size_t sz = 0;
    };

}  // namespace platform


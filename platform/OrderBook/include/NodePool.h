#pragma once

#include <cstddef>

namespace platform {

    template<typename T>
    struct Node {
        Node *prev = nullptr;
        T value{};
        Node *next = nullptr;

        T &get() { return value; }

        const T &get() const { return value; }
    };

    template<typename T, size_t SIZE>
    struct NodePool {
        NodePool() {
            try {
                for (size_t i = 0; i < SIZE; ++i) {
                    this->put(new Node<T>());
                }
            } catch (const std::bad_alloc &) {
                throw std::runtime_error("Failed to allocate node pool");
            }
        }

        Node<T> *get() {
            if (!head) {
                head = new Node<T>();
                return head;
            }
            auto node = head;
            head = head->next;

            node->next = nullptr;
            node->prev = nullptr;

            return node;
        }

        void put(Node<T> *node) {
            if (tail) {
                tail->next = node;
                tail = tail->next;
            } else {
                head = node;
                tail = node;
            }
        }

        Node<T> *head = nullptr;
        Node<T> *tail = nullptr;
    };
}  // namespace platform
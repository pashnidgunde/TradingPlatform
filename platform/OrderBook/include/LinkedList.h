#pragma once
#include <algorithm>

namespace platform {

    template<typename T>
    struct Node {
        Node *prev = nullptr;
        T value;
        Node *next = nullptr;

        explicit Node(T v) : value(std::move(value)) {
        }

    };

    template<typename T, typename Pred = std::less<T>>
    struct LinkedList {
        void insert(T data) {
            if (!head) {
                head = new Node<T>(data);
                return;
            }
            // To do
        }

        Node<T> *head = nullptr;
    };

}


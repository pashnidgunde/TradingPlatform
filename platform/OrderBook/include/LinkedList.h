#pragma once

#include <algorithm>
#include <iostream>

namespace platform {

    template<typename T>
    struct Node {
        Node *prev = nullptr;
        T value{};
        Node *next = nullptr;

        explicit Node(T v) : value(std::move(v)) {
        }

        T& get() { return value; }
    };

    template<typename T, typename Comp = std::less<T>>
    struct LinkedList {
    public:
        class Iterator {
        public:
            Iterator() = delete;
            Iterator(Node<T>* other) {
                node = other;
            }
//            bool operator!=(const Iterator& other) const {
//                return node != other.node;
//            }
//            bool operator==(const Iterator& other) const {
//                return node == other.node;
//            }
            Iterator (const Iterator& other) {
                node = other.node;
            }
            Iterator& operator=(const Iterator& other) {
                node = other.node;
                return *this;
            }
            T& operator*() const {
                return node->get();
            }
            Iterator& operator++() {
                node = node->next;
                return *this;
            }
            Iterator operator++(int) {
                Iterator old = *this;
                node = node->next;
                return old;
            }
            Node<T>* operator ->() const {
                return node;
            }

            operator Node<T>* () const {
                return node;
            }
        private:
            Node<T>* node = nullptr;
        };

        class ReverseIterator {
        public:
            ReverseIterator() = delete;
            ReverseIterator(Node<T>* other) {
                node = other;
            }
//            bool operator!=(const ReverseIterator& other) const {
//                return node != other.node;
//            }
//            bool operator==(const ReverseIterator& other) const {
//                return node == other.node;
//            }
            ReverseIterator (const ReverseIterator& other) {
                node = other.node;
            }
            ReverseIterator& operator=(const ReverseIterator& other) {
                node = other.node;
                return *this;
            }
            T& operator*() const {
                return node->get();
            }
            ReverseIterator& operator++() {
                node = node->prev;
                return *this;
            }
            ReverseIterator operator++(int) {
                ReverseIterator old = *this;
                node = node->prev;
                return old;
            }
            Node<T>* operator ->() const {
                return node;
            }

            operator Node<T>* () const {
                return node;
            }
        private:
            Node<T>* node = nullptr;
        };

        bool isEmpty() const {
            return head == nullptr;
        }

        void insert(const T &data) {
            auto node = new Node<T>(data);
            if (!head) {
                head = node;
                tail = head;
                sz = 1;
                return;
            }
            this->comparedInsert(node);
            sz++;
        }

        size_t size() const {
            return sz;
        }

        Iterator begin() { return Iterator(head); };
        Iterator end() { return Iterator(nullptr); }
        ReverseIterator rbegin() { return ReverseIterator(tail); }
        ReverseIterator rend() { return ReverseIterator(nullptr); }

    private:
        void comparedInsert(Node<T> *nn) {
            // should insert at start ?
            if (compare(nn->value, head->value)) {
                nn->next = head;
                head->prev = nn;
                head = nn;
                return;
            }

            // should insert at end ?
            if (compare(tail->value, nn->value)) {
                tail->next = nn;
                nn->prev = tail;
                tail = nn;
                return;
            }

            auto it = begin();
            while (it) {
                if (compare(nn->value, it->value)) {
                    nn->next = it;
                    nn->prev = it->prev;
                    nn->prev->next = nn;
                    it->prev = nn;
                    return;
                }
                it = it->next;
            }
        }

    private:
        Node<T> *head = nullptr;
        Node<T> *tail = nullptr;
        size_t sz = 0;
        Comp compare{};
    };

}


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
        using NodePtr = Node<T>* ;

        class Iterator {
        public:
            Iterator() = delete;
            Iterator(NodePtr  other) {
                node = other;
            }
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
            NodePtr  operator ->() const {
                return node;
            }

            operator NodePtr  () const {
                return node;
            }
        private:
            NodePtr  node = nullptr;
        };

        class ReverseIterator {
        public:
            ReverseIterator() = delete;
            ReverseIterator(NodePtr  other) {
                node = other;
            }

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
            NodePtr operator ->() const {
                return node;
            }

            operator NodePtr  () const {
                return node;
            }
        private:
            NodePtr  node = nullptr;
        };

        bool isEmpty() const {
            return head == nullptr;
        }

        auto insert(const T &data) {
            auto node = new Node<T>(data);
            if (!head) {
                head = node;
                tail = head;
                sz = 1;
                return;
            }
            this->comparedInsert(node);
            sz++;
            return node;
        }

        size_t size() const {
            return sz;
        }

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
        NodePtr head = nullptr;
        NodePtr tail = nullptr;
        size_t sz = 0;
        Comp compare{};
    };

}


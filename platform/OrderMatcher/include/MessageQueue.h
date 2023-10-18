#pragma once

#include <thread>
#include <unordered_map>
#include "NewMessageHandler.h"
#include "TSQueue.h"

namespace platform {
    struct MessageQueue {
        MessageQueue() {
            consumerThread = std::thread{&MessageQueue::deque, this};
        }

        void enqueue(const Message *msg) {
            q.push(*msg);
        }

        [[noreturn]] void deque() {
            Message msg;
            while (true) {
                msg = q.pop();
                handler.onIncoming(msg);
                // SEND PENDING STATUS BACK
            }
        }

        ~MessageQueue() {
            consumerThread.join();
        }

        TSQueue<Message> q;
        std::thread consumerThread{};
        NewMessageHandler handler;
    };
}

#pragma once

#include <thread>
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

        void deque() {
            Message msg = q.pop();
            handler.onIncoming(msg);
            // SEND PENDING STATUS BACK
        }

        ~MessageQueue() {
            consumerThread.join();
        }

        TSQueue<Message> q;
        std::thread consumerThread{};
        NewMessageHandler handler;
    };
}

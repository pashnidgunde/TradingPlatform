#pragma once

#include <thread>
#include <unordered_map>

#include "MessageHandler.h"


namespace platform {
    struct MessageQueue {
        MessageQueue() {
            consumerThread = std::thread{&MessageQueue::consume, this};
        }

        void incoming(const Message *msg) {
            q.push(*msg);
        }

        void consume() {
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
        MessageHandler handler;
    };




}

#pragma once

#include <array>
#include <unordered_map>
#include "SymbolResolver.h"
#include <boost/lockfree/queue.hpp>
#include "MessageHandler.h"

using namespace boost::lockfree;

namespace platform {
struct MessageQueue {
    MessageQueue() {
        consumerThread = std::thread{&MessageQueue::consume, this};
    }
    void incoming(const Message* msg) {
        q.push(*msg);
    }

    void consume() {
        Message msg;
        while(q.pop(msg)) {
            handler.onIncoming(msg);
            // SEND PENDING STATUS BACK
        }
    }

    ~MessageQueue() {
        consumerThread.join();
    }

    boost::lockfree::queue<Message, boost::lockfree::fixed_sized<false>> q{1024};
    std::thread consumerThread{};
    MessageHandler handler;
    };
}

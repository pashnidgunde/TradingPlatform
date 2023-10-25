#pragma once

#include <iostream>
#include <variant>
#include <atomic>
#include <thread>
#include "TSQueue.h"
#include "OutgoingEvents.h"

template<typename T>
class OrderEventListener {
public:
    OrderEventListener() {
        runner = std::thread(&OrderEventListener::run, this);
        keep_running = true;
    }

    ~OrderEventListener() {
        keep_running = false;
        cv.notify_one();
        if (runner.joinable()) {
            runner.join();
        }
    }

    template<typename Event>
    void onEvent(const Event &msg) {
        std::lock_guard<std::mutex> lock(mtx);
        eventQ.push(msg);
        cv.notify_one();
    }

private:
    void run() {
        auto visitor = [](auto &&event) {
            using EventType = std::decay_t<decltype(event)>;
            if constexpr (std::is_same_v<EventType, std::list<platform::Trade>>) {
                for (const auto &e: event) {
                    std::cout << e << std::endl;
                }
            } else {
                std::cout << event << std::endl;
            }
        };
        std::unique_lock<std::mutex> lock(mtx);

        while (true) {
            cv.wait(lock, [this]() { return !keep_running || !eventQ.empty(); });
            if (!keep_running) break;
            std::visit(visitor, eventQ.front());
            eventQ.pop();
        }
    }
    std::queue<T> eventQ;
    std::mutex mtx;
    std::condition_variable cv;
    std::thread runner{};
    std::atomic<bool> keep_running;
};

#pragma once

#include <iostream>
#include <variant>

#include <thread>
#include "TSQueue.h"
#include "OutgoingEvents.h"

template<typename T>
class OrderEventListner {
public:
    OrderEventListner() {
        runner = std::thread(&OrderEventListner::run, this);
        keep_running = true;
    }

    ~OrderEventListner() {
        keep_running = false;
        cv.notify_one();
        //Join thread
        if (runner.joinable()) {
            runner.join();
        }
    }

    template<typename Event>
    void onEvent(const Event &msg) {
        // Acquire lock
        std::lock_guard<std::mutex> lock(mtx);

        // Add item
        eventQ.push(msg);

        // Notify one thread that
        // is waiting
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

        // acquire lock
        std::unique_lock<std::mutex> lock(mtx);

        while (true) {
            // wait until queue is not empty
            cv.wait(lock, [this]() { return !keep_running || !eventQ.empty(); });

            if (!keep_running) break;

            // retrieve item
            std::visit(visitor, eventQ.front());
            eventQ.pop();
        }
    }

    // Underlying queue
    std::queue<T> eventQ;

    // mutex for thread synchronization
    std::mutex mtx;

    // Condition variable for signaling
    std::condition_variable cv;

    std::thread runner{};
    std::atomic<bool> keep_running;
};

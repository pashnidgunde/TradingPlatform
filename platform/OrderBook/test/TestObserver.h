#pragma once

#include <iostream>
#include <variant>
#include <atomic>
#include <thread>
#include "TSQueue.h"
#include "../include/OutgoingEvents.h"
#include <sstream>

template<typename T>
class TestObserver {
public:
    TestObserver() {
        runner = std::thread(&TestObserver::run, this);
        keep_running = true;
    }

    ~TestObserver() {
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
        auto visitor = [this](auto &&event) {
            using EventType = std::decay_t<decltype(event)>;
            std::stringstream s;
            if constexpr (std::is_same_v<EventType, std::list<platform::Trade>>) {
                for (const auto &e: event) {
                    s << e;
                }
            } else {
                s << event;
            }
            events.emplace_back(s.str());
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
    std::vector<std::string> events;
};


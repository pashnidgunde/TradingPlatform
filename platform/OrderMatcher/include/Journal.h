#pragma once

#include <iostream>
#include <thread>
#include <queue>
#include <mutex>

template<typename T>
class Journal {
public:
    Journal() {
        t = std::thread(&Journal::writeToConsole, this);
    }

    ~Journal() {
        //Join thread
        if (t.joinable()) {
            t.join();
        }
    }

    template<typename LogEvent>
    void log(const LogEvent &msg) {
        std::lock_guard<std::mutex> lg(m);
        events.push_back(msg);
    }

private:
    [[noreturn]] void writeToConsole() {

        auto visitor = [](auto &&event) {
            using EventType = std::decay_t<decltype(event)>;
            if constexpr (std::is_same_v<EventType, std::list<Trade>>) {
                for (const auto &e: event) {
                    std::cout << e << std::endl;
                }
            } else {
                std::cout << event << std::endl;
            }
        };

        while (true) {
            while (events.empty()) {
                std::this_thread::sleep_for(std::chrono::nanoseconds(10));
            }
            m.lock();
            const T &event = events.front();
            events.pop_front();
            m.unlock();

            std::visit(visitor, event);
        }
    }

    std::deque<T> events;
    std::mutex m; // lock/unlock this each time messages is pushed or popped
    std::thread t{};
};

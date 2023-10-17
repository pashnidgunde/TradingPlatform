#pragma once

#include <iostream>
#include <thread>
#include "TSQueue.h"
#include <variant>

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
        events.push(msg);
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
            auto event = events.pop();
            std::visit(visitor, event);
        }
    }

    TSQueue<T> events;
    std::thread t{};
};

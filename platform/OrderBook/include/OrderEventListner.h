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
        t = std::thread(&OrderEventListner::handleEvent, this);
        keep_running = true;
    }

    ~OrderEventListner() {
        keep_running = false;
        //Join thread
        if (t.joinable()) {
            t.join();
        }
    }

    template<typename Event>
    void onEvent(const Event &msg) {
        events.push(msg);
    }

private:
    void handleEvent() {
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
        while(keep_running) {
            auto event = events.pop();
            std::visit(visitor, event);
        }
    }

    TSQueue<T> events;
    std::thread t{};
    bool keep_running = false;
};

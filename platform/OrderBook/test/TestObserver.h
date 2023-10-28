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
    template<typename Event>
    void onEvent(const Event &msg) {
        eventQ.push(msg);
        auto visitor = [this](auto &&event) {
            using EventType = std::decay_t<decltype(event)>;
            std::stringstream s;
            if constexpr (std::is_same_v<EventType, std::list<platform::Trade>>) {
                for (const auto &e: event) {
                    s << e;
                    events.emplace_back(s.str());
                }
            } else {
                s << event;
                events.emplace_back(s.str());
            }

        };
        std::visit(visitor, eventQ.front());
        eventQ.pop();
    }

    const std::vector<std::string>& orderedEvents() const { return events; }

private:

    std::queue<T> eventQ;
    std::vector<std::string> events;
};


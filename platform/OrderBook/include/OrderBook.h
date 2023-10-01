#pragma once

#include "MatchingFields.h"
#include "LinkedList.h"
#include "Order.h"
#include <unordered_map>

namespace platform{
    struct OrderBook {

        bool isEmpty() const {
            return book.empty();
        }

        auto& buyOrders(const std::string& symbol) {
            return book[symbol].first;
        }

        auto& sellOrders(const std::string& symbol) {
            return book[symbol].second;
        }

        void addOrder(const Order& order) {
            const std::string& symbol = order.symbol;
            auto & orders = (order.side == 'B') ?
                            buyOrders(symbol) : sellOrders(symbol);
            orders.insert(std::move(order.mf));
        }

        using MatchFieldsType = platform::LinkedList<platform::MatchingFields>;
        std::unordered_map<std::string,std::pair<MatchFieldsType,MatchFieldsType>> book;
    };
}


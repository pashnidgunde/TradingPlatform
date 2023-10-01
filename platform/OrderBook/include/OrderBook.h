#pragma once
#include "Order.h"
#include "LinkedList.h"

namespace platform{
    template<typename OrderType>
    struct OrderBook {
        bool isEmpty() const {
            return buyOrders.isEmpty() && sellOrders.isEmpty();
        }

        platform::LinkedList<OrderType, std::greater<OrderType>> buyOrders;
        platform::LinkedList<OrderType /*, std::less<OrderType> */> sellOrders;
    };

}
#pragma once
#include "Order.h"
#include "LinkedList.h"

namespace platform{
    template<typename OrderType>
    struct OrderBook {
        bool isEmpty() const {
            return buyOrders.empty() && sellOrders.empty();
        }

        platform::LinkedList<OrderType> buyOrders;
        platform::LinkedList<OrderType> sellOrders;
    };

}
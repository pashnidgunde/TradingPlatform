#pragma once
#include "Order.h"
#include "LinkedList.h"

namespace platform{
    template<typename OrderType>
    struct OrderBook {

        bool isEmpty() const {
            return true;
        }

        platform::LinkedList<OrderType> orders;
    };

}
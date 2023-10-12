#pragma once

#include <cstring>
#include <cstdint>
#include "OrderBookFields.h"

struct Side {
    static constexpr char BUY = 'B';
    static constexpr char SELL = 'S';
};

namespace platform {
    struct Order {

        Order(int uId, int oId, int p, int q, char s, uint16_t sym)
                : mf(uId, oId, p, q), side(s), symbol(sym) {
        }

        platform::OrderBookFields mf;
        char side = Side::BUY;
        uint16_t symbol = -1;
    };

    static_assert(sizeof(Order) == 20);
}  // namespace platform


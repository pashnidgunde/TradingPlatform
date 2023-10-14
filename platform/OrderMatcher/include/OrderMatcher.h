#pragma once

#include <array>
#include <unordered_map>
#include "SymbolResolver.h"
#include "OrderBook.h"


namespace platform {
struct OrderBookHandler {
    void onEvent() {

    }


        SymbolResolver symbolResolver;
        OrderBook<1024> book;
    };
}

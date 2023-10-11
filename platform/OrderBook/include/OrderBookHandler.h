#pragma once

#include <array>
#include <cstdint>
#include "OrderBookLL.h"


namespace platform {
    struct SymbolResolution {
        uint16_t resolve(const std::string &symbol) {
            if (symbolToId.find(symbol) == symbolToId.end()) {
                symbolToId[symbol] = ++sid;
            }
            return symbolToId[symbol];
        }

        std::unordered_map<std::string, uint16_t> symbolToId;
        uint16_t sid = 0;
    };

    struct OrderBookHandler {

        SymbolResolution s;
        OrderBookLL<1024> book;
    };
}

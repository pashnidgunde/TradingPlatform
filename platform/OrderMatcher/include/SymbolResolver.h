#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include <stdexcept>

struct SymbolResolver {
    SymbolResolver(const uint16_t m, const uint16_t x) :
        lowRange(m),
        highRange(x)
    {
    }

    uint16_t resolve(const std::string &symbol) {
        if (sid == highRange) {
            throw std::runtime_error("Unsupported symbol");
        }

        if (symbolToId.find(symbol) == symbolToId.end()) {
            symbolToId[symbol] = ++sid;
        }
        return symbolToId[symbol];
    }

    std::unordered_map<std::string, uint16_t> symbolToId;
    uint16_t sid = 0;
    uint16_t lowRange = 0;
    uint16_t highRange = 1024;

};

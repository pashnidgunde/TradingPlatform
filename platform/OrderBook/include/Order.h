#pragma once

#include <cstring>
#include "MatchingFields.h"

// Assumes that symbol length is 32 bit
#pragma pack(push, 1)
namespace platform {
    struct Order {
        Order(int uId, int oId, int p, int q, char s, char *sym) :
                mf(uId, oId, p, q),
                side(s) {
            memcpy(symbol, sym, sizeof symbol);
        }

        platform::MatchingFields mf;
        char side = 'B';
        char symbol[32]{};
    };
    static_assert(sizeof(Order) == 49);
}

#pragma pack(pop)

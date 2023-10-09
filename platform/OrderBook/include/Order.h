#pragma once

#include <cstring>
#include "OrderBookFields.h"

// Assumes that symbol length is 32 bit
#pragma pack(push, 1)
namespace platform {
struct Order {
  static constexpr char BUY = 'B';
  static constexpr char SELL = 'S';

  Order(int uId, int oId, int p, int q, char s, char* sym)
      : mf(uId, oId, p, q), side(s) {
    memcpy(symbol, sym, sizeof symbol);
  }

  platform::OrderBookFields mf;
  char side = 'B';
  char symbol[32]{};
};
static_assert(sizeof(Order) == 49);
}  // namespace platform

#pragma pack(pop)

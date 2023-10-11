#pragma once

#include <cstring>
#include <cstdint>
#include "OrderBookFields.h"

namespace platform {
struct Order {
  static constexpr char BUY = 'B';
  static constexpr char SELL = 'S';

  Order(int uId, int oId, int p, int q, char s, uint16_t sym)
      : mf(uId, oId, p, q), side(s), sid(sym) {
  }

  platform::OrderBookFields mf;
  char side = 'B';
  uint16_t sid = -1;
};
static_assert(sizeof(Order) == 20);
}  // namespace platform


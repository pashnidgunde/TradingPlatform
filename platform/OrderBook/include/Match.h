#pragma once
#include "OrderBookFields.h"

namespace platform {
struct Match {
  OrderIdentifier buyer;
  OrderIdentifier seller;
  int price;
  int qty;

  Match(OrderIdentifier b, OrderIdentifier s, int price, int qty)
      : buyer(std::move(b)), seller(std::move(s)), price(price), qty(qty) {}
};
} // namespace platform

#pragma once
#include "OrderBookFields.h"

namespace platform {
struct Match {
  OrderIdentifier buyer;
  OrderIdentifier seller;
  int price;
  int qty;

  Match(OrderIdentifier b, OrderIdentifier s, int price, int qty)
      : buyer(b), seller(s), price(price), qty(qty) {}
};
}  // namespace platform

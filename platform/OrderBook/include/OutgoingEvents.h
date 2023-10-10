#pragma once
#include "OrderBookFields.h"

namespace platform {
struct Ack {
  static constexpr char value = 'A';
  OrderIdentifier sender;
};

struct CancelAck {
  static constexpr char value = 'C';
  OrderIdentifier sender;
};

struct Trade {
  static constexpr char value = 'T';
  OrderIdentifier buyer;
  OrderIdentifier seller;
  int price;
  int qty;

  Trade(OrderIdentifier b, OrderIdentifier s, int price, int qty)
      : buyer(b), seller(s), price(price), qty(qty) {}
};

struct Tob {
  static constexpr char value = 'B';
  char side = 'B';  // or 'S'
  int price;
  int totalQty;
};

}  // namespace platform

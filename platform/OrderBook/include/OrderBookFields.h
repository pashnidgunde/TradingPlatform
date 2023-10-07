#pragma once

namespace platform {
struct OrderBookFields {
  int userId = 0;
  int orderId = 0;
  int price = 0;
  int qty = 0;

  OrderBookFields() = default;
  OrderBookFields(int uId, int oId, int p, int q)
      : userId(uId), orderId(oId), price(p), qty(q) {}
  OrderBookFields(const OrderBookFields &other) = default;

  bool operator<(const OrderBookFields &rhs) const { return price < rhs.price; }

  bool operator==(const OrderBookFields &rhs) const {
    return userId == rhs.userId && orderId == rhs.orderId &&
           price == rhs.price && qty == rhs.qty;
  }

  bool operator!=(const OrderBookFields &rhs) const { return !(rhs == *this); }

  bool operator>(const OrderBookFields &rhs) const { return rhs < *this; }
};

static_assert(sizeof(OrderBookFields) == 16);
} // namespace platform

#pragma once

#include <ostream>
#include "WireFormat.h"

namespace platform {


struct OrderBookFields {
  OrderIdentifier oi;
  int price = 0;
  int qty = 0;

  bool operator==(const OrderBookFields& rhs) const {
    return price == rhs.price;
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  const OrderBookFields& fields) {
    os << "oi: " << fields.oi << " price: " << fields.price
       << " qty: " << fields.qty;
    return os;
  }

  OrderBookFields() = default;
  OrderBookFields(int uId, int oId, int p, int q)
      : oi{uId, oId}, price(p), qty(q) {}

  bool operator<(const OrderBookFields& rhs) const { return price < rhs.price; }

  bool operator>(const OrderBookFields& rhs) const { return rhs < *this; }
};

static_assert(sizeof(OrderBookFields) == 16);
}  // namespace platform

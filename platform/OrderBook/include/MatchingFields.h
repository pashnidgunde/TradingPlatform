#pragma once

namespace platform {
struct MatchingFields {
  int userId = 0;
  int orderId = 0;
  int price = 0;
  int qty = 0;

  MatchingFields() = default;
  MatchingFields(int uId, int oId, int p, int q)
      : userId(uId), orderId(oId), price(p), qty(q) {}
  MatchingFields(const MatchingFields &other) = default;

  bool operator<(const MatchingFields &rhs) const { return price < rhs.price; }

  bool operator==(const MatchingFields &rhs) const {
    return userId == rhs.userId && orderId == rhs.orderId &&
           price == rhs.price && qty == rhs.qty;
  }

  bool operator!=(const MatchingFields &rhs) const { return !(rhs == *this); }

  bool operator>(const MatchingFields &rhs) const { return rhs < *this; }
};

static_assert(sizeof(MatchingFields) == 16);
} // namespace platform

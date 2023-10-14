#pragma once
#include <list>


using Price = int;
using OrderId = int;
using UserId = int;
using Qty = int;
using SymbolId = int;

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
  OrderIdentifier buyer{};
  OrderIdentifier seller{};
  int price{};

    Trade(const OrderIdentifier &buyer, const OrderIdentifier &seller, int price, int qty) :
        buyer(buyer),
        seller(seller),
        price(price), qty(qty) {}
    int qty{};
};
using Trades = std::list<platform::Trade>;

template<char SIDE>
struct TopOfBook {
    Price price{};

    bool operator==(const TopOfBook &rhs) const {
        return price == rhs.price &&
               qty == rhs.qty;
    }

    bool operator!=(const TopOfBook &rhs) const {
        return !(rhs == *this);
    }

    Qty qty{};
    static constexpr char side = SIDE;
};

}  // namespace platform

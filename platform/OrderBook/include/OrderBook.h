#pragma once

#include "LinkedList.h"
#include "OrderBookFields.h"
#include "Order.h"
#include <unordered_map>

namespace platform {
struct OrderBook {

  [[nodiscard]] bool isEmpty() const { return book.empty(); }

  auto &buyOrders(const std::string &symbol) { return book[symbol].first; }

  auto &sellOrders(const std::string &symbol) { return book[symbol].second; }

  auto addBuy(const std::string& symbol, OrderBookFields fields) {
    return buyOrders(symbol).insert(fields);
  }

  auto addSell(const std::string& symbol, const OrderBookFields& fields) {
      return sellOrders(symbol).insert(fields);
  }

  using MatchFieldsType = platform::LinkedList<platform::OrderBookFields>;
  std::unordered_map<std::string, std::pair<MatchFieldsType, MatchFieldsType>>
      book;
};
} // namespace platform

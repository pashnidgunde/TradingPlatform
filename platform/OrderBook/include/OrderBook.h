#pragma once

#include <unordered_map>
#include <queue>
#include <functional>
#include <array>
#include <optional>

#include "LinkedList.h"
#include "Order.h"

using Price = int;
using OrderId = int;
using UserId = int;
using Qty = int;
using SymbolId = int;

using namespace platform;

struct OrderFields {
    UserId userId{};
    OrderId orderId{};
    Qty qty{};
};

struct NewOrder {
    OrderFields of;
    SymbolId symbol{};
    Price price{};

    NewOrder(UserId u, OrderId o, Qty q, SymbolId s, Price p) :
            of{u, o, q}, symbol(s), price(p) {
    }
};

struct OrdersBySymbolId {

    template<char SIDE, typename T>
    struct OrdersBySide {
        [[nodiscard]] Price topPrice() const { return static_cast<const T *>(this)->pricePriority.top(); }

        bool exists(const Price price) {
            auto &map = static_cast<const T *>(this)->ordersByPrice;
            return map.find(price) != map.end();
        };

        [[nodiscard]] const auto &orders(Price price) const {
            return static_cast<const T *>(this)->ordersByPrice.at(price);
        }

        auto &mutable_orders(Price price) {
            return static_cast<T *>(this)->ordersByPrice[price];
        }

        [[nodiscard]] bool empty() const { return static_cast<const T *>(this)->ordersByPrice.empty(); }

        [[nodiscard]] size_t size() const {
            size_t size = 0;
            for (const auto &kv: static_cast<const T *>(this)->ordersByPrice) {
                size += kv.second.size();
            }
            return size;
        }

        [[nodiscard]] const OrderFields &top() const {
            return this->orders(topPrice()).front();
        }

        LinkedList<OrderFields>::NodePtr insert(const NewOrder &order) {
            if (!exists(order.price)) {
                static_cast<T *>(this)->pricePriority.emplace(order.price);
            }
            auto node = mutable_orders(order.price).insert(order.of);
            return node;
        }
    };

    struct BuyOrders : public OrdersBySide<Side::BUY, BuyOrders> {
        std::priority_queue<Price, std::vector<Price>, std::less<>> pricePriority;
        std::unordered_map<Price, LinkedList<OrderFields>> ordersByPrice;
    };

    struct SellOrders : public OrdersBySide<Side::SELL, SellOrders> {
        std::priority_queue<Price, std::vector<Price>, std::greater<>> pricePriority;
        std::unordered_map<Price, LinkedList<OrderFields>> ordersByPrice;
    };

    BuyOrders buyOrders;
    SellOrders sellOrders;
};

template<char SIDE>
struct TopOfBook {
    static constexpr char side = SIDE;
    OrderFields fields;

    explicit TopOfBook(const OrderFields &fields) : fields(fields) {}
};


template<size_t SIZE = 1024>
struct OrderBook {

    [[nodiscard]] bool isEmpty(SymbolId symbol) const {
        return book[symbol].buyOrders.empty() && book[symbol].sellOrders.empty();
    }

    auto addBuy(const NewOrder &order) {
        return book[order.symbol].buyOrders.insert(order);
    }

    auto addSell(const NewOrder &order) {
        return book[order.symbol].sellOrders.insert(order);
    }

    auto buyTop(SymbolId symbol) const {
        return !isEmpty(symbol) ?
               std::optional<TopOfBook<Side::BUY>>{book[symbol].buyOrders.top()} :
               std::nullopt;
    }

    auto sellTop(SymbolId symbol) const {
        return !isEmpty(symbol) ?
               std::optional<TopOfBook<Side::SELL>>{book[symbol].sellOrders.top()} :
               std::nullopt;
    }

    auto &buyOrders(SymbolId symbol) const {
        return book[symbol].buyOrders;
    }

    auto &sellOrders(SymbolId symbol) const {
        return book[symbol].sellOrders;
    }

    std::array<OrdersBySymbolId, SIZE> book;
};
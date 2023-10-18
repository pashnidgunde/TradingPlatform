#pragma once

#include <functional>
#include <array>
#include <optional>
#include <map>
#include <unordered_map>
#include <list>
#include <functional>

#include "WireFormat.h"
#include "OutgoingEvents.h"

using namespace std;

struct OrderIdentifierHasher {
    std::size_t operator()(const OrderIdentifier &oi) const {
        return ((std::hash<UserId>()(oi.userId))
                ^ (std::hash<OrderId>()(oi.orderId) << 1));
    }
};

using FIFOOrderQueue = std::list<Order>;

struct OrdersBySymbolId {
    template<char SIDE, typename T>
    struct OrdersBySide {

        [[nodiscard]] const auto &ordersAtPrice(Price price) const {
            return static_cast<const T *>(this)->orders.at(price);
        }

        auto &mutableOrdersAtPrice(Price price) {
            return static_cast<T *>(this)->orders[price];
        }

        [[nodiscard]] bool isEmpty() const { return static_cast<const T *>(this)->orders.empty(); }

        auto insert(const Order &order) {
            auto &ll = mutableOrdersAtPrice(order.price);
            auto node = ll.emplace_back(order);
            return node;
        }

        [[nodiscard]] size_t size(Price price) const {
            auto &ll = ordersAtPrice(price);
            return ll.empty() ? 0 : ll.size();
        }

        [[nodiscard]] const auto &getAll() const {
            return static_cast<const T *>(this)->orders;
        }

        [[nodiscard]] auto &mutableAll() {
            return static_cast<T *>(this)->orders;
        }
    };

    struct BuyOrders : public OrdersBySide<SIDE_BUY, BuyOrders> {
        std::map<Price, std::list<Order>, std::greater<>> orders;
    };

    struct SellOrders : public OrdersBySide<SIDE_SELL, SellOrders> {
        std::map<Price, std::list<Order>, std::less<>> orders;
    };

    BuyOrders buyOrders;
    SellOrders sellOrders;

};

template<size_t SIZE = 1024>
struct OrderBook {

    const auto &buyOrders(SymbolId id) const {
        return book[id].buyOrders;
    }

    auto &sellOrders(SymbolId id) const {
        return book[id].sellOrders;
    }

    auto &mutableBuyOrders(SymbolId id) {
        return book[id].buyOrders.mutableAll();
    }

    auto &mutableSellOrders(SymbolId id) {
        return book[id].sellOrders.mutableAll();
    }

    [[nodiscard]] bool isEmpty(SymbolId symbol) const {
        return buyOrders(symbol).isEmpty() && book[symbol].sellOrders.isEmpty();
    }

    [[maybe_unused]] auto addBuy(const Order &order) {
        auto &ll = mutableBuyOrders(order.symbol.id)[(order.price)];
        auto it = ll.insert(ll.end(), order);
        orderIdToNodeMap[{order.oi.userId, order.oi.orderId}] = it;
        return it;
    }

    [[maybe_unused]] auto addSell(const Order &order) {
        auto &ll = mutableSellOrders(order.symbol.id)[(order.price)];
        auto it = ll.insert(ll.end(), order);
        orderIdToNodeMap[{order.oi.userId, order.oi.orderId}] = it;
        return it;
    }

    void cross(std::list<Order> &buys, std::list<Order> &sells, const Price matchPrice, platform::Trades &trades) {
        auto adjustOpenQty = [](Order &o, int qty) { o.qty -= qty; };

        auto completelyFilled = [](Order &o) { return o.qty == 0; };

        auto matchQty = 0;
        for (auto &buy: buys) {
            for (auto &sell: sells) {
                matchQty = std::min(buy.qty, sell.qty);
                trades.emplace_back(buy.oi, sell.oi, matchPrice, matchQty);
                adjustOpenQty(buy, matchQty);
                adjustOpenQty(sell, matchQty);
                if (completelyFilled(buy)) {
                    break;
                }
            }
        }

        auto removeCompletelyFilled = [&]() {
            auto zeroRemainingQty = [](Order &order) {
                return order.qty == 0;
            };

            buys.remove_if(zeroRemainingQty);
            sells.remove_if(zeroRemainingQty);
        };
        removeCompletelyFilled();
    }

    platform::Trades tryCross(const SymbolId id) {
        auto canCross = [](const auto &buyPrice, const auto &sellPrice) {
            return buyPrice >= sellPrice;
        };

        platform::Trades trades;
        auto matchPrice = 0;
        for (auto &buyPair: mutableBuyOrders(id)) {
            for (auto &sellPair: mutableSellOrders(id)) {
                if (!canCross(buyPair.first, sellPair.first))
                    break;
                matchPrice = std::min(buyPair.first, sellPair.first);
                cross(buyPair.second, sellPair.second, matchPrice, trades);
            }
        }

        auto removeEmptyPriceLevels = [&](SymbolId id) {
            auto removeEmptyPriceLevel = [](auto &ordersByPriceLevel) {
                for (auto it = ordersByPriceLevel.cbegin(); it != ordersByPriceLevel.cend();) {
                    if (it->second.size() == 0) {
                        ordersByPriceLevel.erase(it++);
                    } else {
                        // short circuit as levels are ordered
                        break;
                    }
                }
            };

            removeEmptyPriceLevel(mutableBuyOrders(id));
            removeEmptyPriceLevel(mutableSellOrders(id));
        };
        removeEmptyPriceLevels(id);

        return trades;
    }

    void flush() {
        auto clearAll = [](auto &mutableOrders) {
            for (auto &pair: mutableOrders) {
                pair.second.clear();
            }
            mutableOrders.clear();
        };

        for (size_t i = 0; i < SIZE; ++i) {
            clearAll(mutableBuyOrders(i));
            clearAll(mutableSellOrders(i));
        }
    }


    void cancel(const OrderIdentifier &oi) {
        auto it = orderIdToNodeMap.find(oi);
        if (it == orderIdToNodeMap.end()) {
            // cancel reject
            // too late to cancel ?
            return;
        }
        auto &iter = it->second;
        auto &order = *iter;
        if (order.side == SIDE_BUY) {
            auto &orderMap = mutableBuyOrders(order.symbol.id);
            auto &ll = orderMap[order.price];
            ll.erase(iter);
            if (ll.empty()) {
                orderMap.erase(order.price);
            }
        } else {
            auto &orderMap = mutableSellOrders(order.symbol.id);
            auto &ll = orderMap[order.price];
            ll.erase(iter);
            if (ll.empty()) {
                orderMap.erase(order.price);
            }
        }
    }

    std::array<OrdersBySymbolId, SIZE> book;
    std::unordered_map<OrderIdentifier, std::list<Order>::iterator, OrderIdentifierHasher> orderIdToNodeMap;
};
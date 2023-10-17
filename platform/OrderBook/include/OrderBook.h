#pragma once

#include <functional>
#include <array>
#include <optional>
#include <map>

#include "WireFormat.h"
#include "LinkedList.h"
#include "OutgoingEvents.h"

using namespace platform;

struct OrderIdentifierHasher {
    std::size_t operator()(const OrderIdentifier &oi) const {
        return ((std::hash<UserId>()(oi.userId))
                ^ (std::hash<OrderId>()(oi.orderId) << 1));
    }
};

using FIFOOrderQueue = LinkedList<Order>;

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

        FIFOOrderQueue::NodePtr insert(const Order &order) {
            auto &ll = mutableOrdersAtPrice(order.price);
            auto node = ll.insert(order);
            return node;
        }

        [[nodiscard]] size_t size(Price price) const {
            return (ordersAtPrice(price).isEmpty()) ? 0 : ordersAtPrice(price).size();
        }

        [[nodiscard]] const auto &getAll() const {
            return static_cast<const T *>(this)->orders;
        }

        [[nodiscard]] auto &mutableAll() {
            return static_cast<T *>(this)->orders;
        }

        [[nodiscard]] std::optional<TopOfBook<SIDE>> top() const {
            if (isEmpty()) return std::nullopt;
            auto it = static_cast<const T *>(this)->orders.begin();
            return TopOfBook<SIDE>{it->second.front()};
        }
    };

    struct BuyOrders : public OrdersBySide<SIDE_BUY, BuyOrders> {
        std::map<Price, FIFOOrderQueue, std::greater<>> orders;
    };

    struct SellOrders : public OrdersBySide<SIDE_SELL, SellOrders> {
        std::map<Price, FIFOOrderQueue, std::less<>> orders;
    };

    BuyOrders buyOrders;
    SellOrders sellOrders;

};

template<size_t SIZE = 1024>
struct OrderBook {

    [[nodiscard]] bool isEmpty(SymbolId symbol) const {
        return book[symbol].buyOrders.isEmpty() && book[symbol].sellOrders.isEmpty();
    }

    [[maybe_unused]] auto addBuy(const Order &order) {
        auto node = book[order.symbol.id].buyOrders.insert(order);
        orderIdToNodeMap[{order.oi.userId, order.oi.orderId}] = node;
    }

    [[maybe_unused]] auto addSell(const Order &order) {
        auto node = book[order.symbol.id].sellOrders.insert(order);
        orderIdToNodeMap[{order.oi.userId, order.oi.orderId}] = node;
        return node;
    }

    auto buyTop(SymbolId id) const {
        return book[id].buyOrders.top();
    }

    auto sellTop(SymbolId id) const {
        return book[id].sellOrders.top();
    }

    auto &buyOrders(SymbolId id) const {
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

    void cross(FIFOOrderQueue &buys, FIFOOrderQueue &sells, const Price matchPrice, Trades &trades) {
        auto adjustOpenQty = [](Order &o, int qty) { o.qty -= qty; };

        auto completelyFilled = [](Order &o) { return o.qty == 0; };

        auto matchQty = 0;
        for (auto b = buys.begin(); b != buys.end(); ++b) {
            for (auto s = sells.begin(); s != sells.end(); ++s) {
                auto &buy = b->get();
                auto &sell = s->get();
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
            auto zeroRemainingQty = [](FIFOOrderQueue::NodePtr node) {
                return node->get().qty == 0;
            };

            buys.removeIf(zeroRemainingQty);
            sells.removeIf(zeroRemainingQty);
        };
        removeCompletelyFilled();
    }

    platform::Trades tryCross(const SymbolId id) {
        auto canCross = [](const auto &buyPrice, const auto &sellPrice) {
            return buyPrice >= sellPrice;
        };

        Trades trades;
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

        latestBuyTop.flush();
        latestSellTop.flush();
    }


    using TopOfBookPair = std::pair<std::optional<TopOfBook<SIDE_BUY>>, std::optional<TopOfBook<SIDE_SELL>>>;

    void cancel(const OrderIdentifier &oi) {

        auto it = orderIdToNodeMap.find(oi);
        if (it == orderIdToNodeMap.end()) {
            // cancel reject
            // too late to cancel ?
            return;
        }
    }

    TopOfBookPair topOfBook(const SymbolId id) {
        TopOfBookPair pair;

        auto newBuyTop = buyTop(id);
        if (newBuyTop.has_value()) {
            auto &value = newBuyTop.value();
            if (value != latestBuyTop) {
                latestBuyTop = value;
                pair.first = latestBuyTop;
            }
        }

        auto newSellTop = sellTop(id);
        if (newSellTop.has_value()) {
            auto &value = newSellTop.value();
            if (value != latestSellTop) {
                latestSellTop = value;
                pair.second = latestSellTop;
            }
        }

        return pair;
    }

    auto find(const OrderIdentifier &oi) {
        auto it = orderIdToNodeMap.find(oi);
        return (it != orderIdToNodeMap.end()) ? it->second : nullptr;
    }

    TopOfBook<SIDE_BUY> latestBuyTop;
    TopOfBook<SIDE_SELL> latestSellTop;
    std::array<OrdersBySymbolId, SIZE> book;
    std::unordered_map<OrderIdentifier, FIFOOrderQueue::NodePtr, OrderIdentifierHasher> orderIdToNodeMap;
};
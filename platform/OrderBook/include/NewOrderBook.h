#pragma once

#include <functional>
#include <array>
#include <optional>
#include <map>
#include <unordered_map>
#include <list>
#include <functional>
#include <variant>

#include "WireFormat.h"
#include "OutgoingEvents.h"
#include "OrderEventListner.h"
#include <iostream>

using namespace std;

struct OrderIdentifierHasher {
    std::size_t operator()(const OrderIdentifier &oi) const {
        return ((std::hash<UserId>()(oi.userId))
                ^ (std::hash<OrderId>()(oi.orderId) << 1));
    }
};

struct OrderBook {

    template<char SIDE>
    void addOrder(const Order &order);

    template<>
    void addOrder<SIDE_BUY>(const Order &order) {
        auto &ordersBySymbol = buyOrdersBySymbol[order.symbol.id];
        auto &ll = ordersBySymbol[order.price];
        auto it = ll.insert(ll.end(), order);
        orderIdToNodeMap[{order.oi.userId, order.oi.orderId}] = it;
    }

    template<>
    void addOrder<SIDE_SELL>(const Order &order) {
        auto &ordersBySymbol = sellOrdersBySymbol[order.symbol.id];
        auto &ll = ordersBySymbol[order.price];
        auto it = ll.insert(ll.end(), order);
        orderIdToNodeMap[{order.oi.userId, order.oi.orderId}] = it;
    }

    void addOrder(const Order &order) {
        if (order.side == SIDE_BUY) {
            addOrder<SIDE_BUY>(order);
        } else {
            addOrder<SIDE_SELL>(order);
        }
    }

    template<char SIDE>
    void addOrder(Order &&order);

    template<>
    void addOrder<SIDE_BUY>(Order &&order) {
        auto &ordersBySymbol = buyOrdersBySymbol[order.symbol.id];
        auto &ll = ordersBySymbol[order.price];
        auto it = ll.insert(ll.end(), std::move(order));
        orderIdToNodeMap[{order.oi.userId, order.oi.orderId}] = it;
    }

    template<>
    void addOrder<SIDE_SELL>(Order &&order) {
        auto &ordersBySymbol = sellOrdersBySymbol[order.symbol.id];
        auto &ll = ordersBySymbol[order.price];
        auto it = ll.insert(ll.end(), std::move(order));
        orderIdToNodeMap[{order.oi.userId, order.oi.orderId}] = it;
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

            auto zeroRemainingQty = [](const Order &order) {
                return order.qty == 0;
            };

            auto removeZeroQtyOrders = [&](auto &orders) {
                for (std::list<Order>::iterator it = orders.begin(); it != orders.end();) {
                    if (zeroRemainingQty(*it)) {
                        orderIdToNodeMap.erase({it->oi});
                        it = orders.erase(it);
                    } else {
                        break;
                    }
                }
            };

            removeZeroQtyOrders(buys);
            removeZeroQtyOrders(sells);

        };
        removeCompletelyFilled();
    }

    platform::Trades tryCross(const SymbolId &symbol) {
        platform::Trades trades;
        auto canCross = [](const auto &buyPrice, const auto &sellPrice) {
            return buyPrice >= sellPrice;
        };

        if (buyOrdersBySymbol.find(symbol) == buyOrdersBySymbol.end()) return trades;
        if (sellOrdersBySymbol.find(symbol) == sellOrdersBySymbol.end()) return trades;


        for (auto &buysAtPriceLevel: buyOrdersBySymbol.at(symbol)) {
            for (auto &sellsAtPriceLevel: sellOrdersBySymbol.at(symbol)) {
                if (!canCross(buysAtPriceLevel.first, sellsAtPriceLevel.first))
                    break;
                auto matchPrice = std::min(buysAtPriceLevel.first, sellsAtPriceLevel.first);
                cross(buysAtPriceLevel.second, sellsAtPriceLevel.second, matchPrice, trades);
            }
        }

        auto removeEmptyPriceLevel = [&](auto &ordersByPriceLevel) {
            for (auto it = ordersByPriceLevel.cbegin(); it != ordersByPriceLevel.cend();) {
                if (it->second.empty()) {
                    ordersByPriceLevel.erase(it++);
                } else {
                    // short circuit as levels are ordered
                    break;
                }
            }
        };

        auto removeEmptyPriceLevels = [&](SymbolId symbol) {
            removeEmptyPriceLevel(buyOrdersBySymbol.at(symbol));
            removeEmptyPriceLevel(sellOrdersBySymbol.at(symbol));
        };
        removeEmptyPriceLevels(symbol);
        return trades;
    }

    void flush() {
        buyOrdersBySymbol.clear();
        sellOrdersBySymbol.clear();
        orderIdToNodeMap.clear();
    }


    void cancel(const OrderIdentifier &oi) {
        auto it = orderIdToNodeMap.find(oi);
        if (it == orderIdToNodeMap.end()) {
            return;
        }
        auto &iter = it->second;
        auto &order = *iter;
        if (order.side == SIDE_BUY) {
            auto &orderMap = buyOrdersBySymbol.at(order.symbol.id);
            auto price = order.price;
            auto &ll = orderMap.at(price);
            ll.erase(iter);
            if (ll.empty()) {
                orderMap.erase(price);
            }
        } else {
            auto &orderMap = sellOrdersBySymbol.at(order.symbol.id);
            auto price = order.price;
            auto &ll = orderMap[price];
            ll.erase(iter);
            if (ll.empty()) {
                orderMap.erase(price);
            }
        }
    }

    bool isEmpty() const {
        return buyOrdersBySymbol.empty() && sellOrdersBySymbol.empty();
    }

    using BuyOrdersAtPrice = std::map<Price, std::list<Order>, std::greater<>>;
    using SellOrdersAtPrice = std::map<Price, std::list<Order>, std::less<>>;

    std::optional<BuyOrdersAtPrice> buyOrders(SymbolId id) {
        return (buyOrdersBySymbol.find(id) == buyOrdersBySymbol.end()) ?
               std::nullopt : std::make_optional<BuyOrdersAtPrice>(buyOrdersBySymbol.at(id));
    }

    std::optional<SellOrdersAtPrice> sellOrders(SymbolId id) {
        return (sellOrdersBySymbol.find(id) == sellOrdersBySymbol.end()) ?
               std::nullopt : std::make_optional<SellOrdersAtPrice>(sellOrdersBySymbol.at(id));
    }

    std::unordered_map<SymbolId, BuyOrdersAtPrice> buyOrdersBySymbol;
    std::unordered_map<SymbolId, SellOrdersAtPrice> sellOrdersBySymbol;
    std::unordered_map<OrderIdentifier, std::list<Order>::iterator, OrderIdentifierHasher> orderIdToNodeMap;
    OrderEventListner<std::variant<platform::Ack>> listner;
};
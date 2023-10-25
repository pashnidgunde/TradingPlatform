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

using BuyOrdersAtPrice = std::map<Price, std::list<Order *>, std::greater<>>;
using SellOrdersAtPrice = std::map<Price, std::list<Order *>, std::less<>>;

struct tob {
    Order *preTop = nullptr;
    Order *postTop = nullptr;

    template<typename T>
    void top(const T &orders, Order *&o) {
        if (!orders.empty()) {
            if (!orders.begin()->second.empty()) {
                if (!orders.begin()->second.begin()->second.empty()) {
                    o = orders.begin()->second.begin()->second.front();
                }
            }
        }
    }

    template<typename T>
    void pre(const T &orders) {
        this->top(orders, preTop);
    }

    template<typename T>
    void post(const T &orders) {
        this->top(orders, postTop);
    }

    [[nodiscard]] const Order *latest() const {
        return (preTop != postTop) ? postTop : nullptr;
    }
};


class OrderBook {
public:
    void addOrder(Order *order) {
        listner.onEvent(platform::Ack{order->oi});
        tob t;
        std::list<Order *>::iterator it;
        if (order->side == SIDE_BUY) {
            t.pre(buyOrdersBySymbol);
            it = _addOrder(buyOrdersBySymbol, order);
            t.post(buyOrdersBySymbol);
        } else if (order->side == SIDE_SELL) {
            t.pre(sellOrdersBySymbol);
            it = _addOrder(sellOrdersBySymbol, order);
            t.post(sellOrdersBySymbol);
        } else {
            throw std::runtime_error("Unsupported side");
        }
        orderIdToNodeMap[{order->oi.userId, order->oi.orderId}] = it;

        try_cross(order->symbol.id);

        const Order *tob = t.latest();
        if (tob) {
            listner.onEvent(platform::TopOfBook(tob));
        }
    }

    void flush() {
        buyOrdersBySymbol.clear();
        sellOrdersBySymbol.clear();
        orderIdToNodeMap.clear();

        listner.onEvent(Flush{});
    }


    void cancel(const OrderIdentifier &oi) {
        auto it = orderIdToNodeMap.find(oi);
        if (it == orderIdToNodeMap.end()) {
            return;
        }
        auto &iter = it->second;
        auto &order = *iter;
        tob t;
        if (order->side == SIDE_BUY) {
            t.pre(buyOrdersBySymbol);
            auto &orderMap = buyOrdersBySymbol.at(order->symbol.id);
            auto price = order->price;
            auto &ll = orderMap.at(price);
            ll.erase(iter);
            if (ll.empty()) {
                orderMap.erase(price);
            }
            t.post(buyOrdersBySymbol);
        } else {
            t.pre(sellOrdersBySymbol);
            auto &orderMap = sellOrdersBySymbol.at(order->symbol.id);
            auto price = order->price;
            auto &ll = orderMap[price];
            ll.erase(iter);
            if (ll.empty()) {
                orderMap.erase(price);
            }
            t.post(sellOrdersBySymbol);
        }
        const Order *tob = t.latest();
        if (tob) {
            listner.onEvent(platform::TopOfBook(tob));
        }
    }

    bool isEmpty() const {
        return buyOrdersBySymbol.empty() && sellOrdersBySymbol.empty();
    }

    std::optional<BuyOrdersAtPrice> buyOrders(SymbolId id) {
        return (buyOrdersBySymbol.find(id) == buyOrdersBySymbol.end()) ?
               std::nullopt : std::make_optional<BuyOrdersAtPrice>(buyOrdersBySymbol.at(id));
    }

    std::optional<SellOrdersAtPrice> sellOrders(SymbolId id) {
        return (sellOrdersBySymbol.find(id) == sellOrdersBySymbol.end()) ?
               std::nullopt : std::make_optional<SellOrdersAtPrice>(sellOrdersBySymbol.at(id));
    }

private:
    template<typename T>
    std::list<Order *>::iterator _addOrder(T &existing, Order *incoming) {
        auto &ordersBySide = existing[incoming->symbol.id];
        auto &ll = ordersBySide[incoming->price];
        return ll.insert(ll.end(), incoming);
    }

    void cross(std::list<Order *> &buys, std::list<Order *> &sells, const Price matchPrice, platform::Trades &trades) {
        auto adjustOpenQty = [](Order *o, int qty) { o->qty -= qty; };

        auto completelyFilled = [](const Order *o) { return o->qty == 0; };

        auto matchQty = 0;
        for (auto &buy: buys) {
            for (auto &sell: sells) {
                matchQty = std::min(buy->qty, sell->qty);
                trades.emplace_back(buy->oi, sell->oi, matchPrice, matchQty);
                adjustOpenQty(buy, matchQty);
                adjustOpenQty(sell, matchQty);
                if (completelyFilled(buy)) {
                    break;
                }
            }
        }

        auto removeCompletelyFilled = [&]() {
            auto zeroRemainingQty = [](const Order *order) {
                return order->qty == 0;
            };

            auto removeZeroQtyOrders = [&](auto &orders) {
                for (auto it = orders.begin(); it != orders.end();) {
                    if (zeroRemainingQty(*it)) {
                        orderIdToNodeMap.erase({(*it)->oi});
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

    platform::Trades try_cross(const SymbolId &symbol) {
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

        listner.onEvent(trades);
        return trades;
    }


    std::unordered_map<SymbolId, BuyOrdersAtPrice> buyOrdersBySymbol;
    std::unordered_map<SymbolId, SellOrdersAtPrice> sellOrdersBySymbol;
    std::unordered_map<OrderIdentifier, std::list<Order *>::iterator, OrderIdentifierHasher> orderIdToNodeMap;
    OrderEventListner<std::variant<platform::Ack, platform::TopOfBook, platform::Trades, Flush>> listner;
};
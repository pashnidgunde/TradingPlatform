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
#include "OrderEventListener.h"
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
using BookListner = OrderEventListener<std::variant<platform::Ack, platform::TopOfBook, platform::Trades, Flush>>;
struct TopOfBooksRAII {
    template<typename T>
    [[nodiscard]] const Order* top(const T &orders, SymbolId sid) const {
        if (orders.find(sid) != orders.end()) {
            auto &opl = orders.at(id);
            if (!opl.empty()) {
                auto &ll = opl.cbegin()->second;
                if (!ll.empty()) {
                    return ll.front();
                }
            }
        }
        return nullptr;
    }

    TopOfBooksRAII(BookListner& listener, const std::unordered_map<SymbolId, BuyOrdersAtPrice>& allBuyOrders,
                   const std::unordered_map<SymbolId, SellOrdersAtPrice>& allSellOrders,
                    SymbolId id) :
        listener(listener),
        allBuyOrders(allBuyOrders),
        allSellOrders(allSellOrders),
        id(id) {
        preBuyTop = this->top(allBuyOrders,id);
        preSellTop = this->top(allSellOrders, id);
    }

    ~TopOfBooksRAII() {
        postBuyTop = this->top(allBuyOrders, id);
        postSellTop = this->top(allSellOrders, id);

        if (preBuyTop != postBuyTop) {
            listener.onEvent(platform::TopOfBook('B', postBuyTop));
        }
        if (preSellTop != postSellTop) {
            listener.onEvent(platform::TopOfBook('S', postSellTop));
        }
    }

    BookListner& listener;
    const std::unordered_map<SymbolId, BuyOrdersAtPrice>& allBuyOrders;
    const std::unordered_map<SymbolId, SellOrdersAtPrice>& allSellOrders;
    const SymbolId id{};

    const Order *preBuyTop{nullptr};
    const Order *postBuyTop{nullptr};
    const Order *preSellTop{nullptr};
    const Order *postSellTop{nullptr};
};


class OrderBook {
public:

    template<typename T>
    void _add(T& existing, Order *incoming) {
        TopOfBooksRAII t(this->listner, buyOrdersBySymbol, sellOrdersBySymbol, incoming->symbol.id);
        auto &ordersBySide = existing[incoming->symbol.id];
        auto &ll = ordersBySide[incoming->price];
        auto it = ll.insert(ll.end(), incoming);
        orderIdToNodeMap[{incoming->oi.userId, incoming->oi.orderId}] = it;
        try_cross(incoming->symbol.id);
    }

    void addOrder(Order *order) {
        listner.onEvent(platform::Ack{order->oi});
        if (order->side == SIDE_BUY) {
            _add(buyOrdersBySymbol, order);
        } else if (order->side == SIDE_SELL) {
            _add(sellOrdersBySymbol, order);
        } else {
            throw std::runtime_error("Unsupported side");
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

        if (order->side == SIDE_BUY) {
            auto &orderMap = buyOrdersBySymbol.at(order->symbol.id);
            auto price = order->price;
            auto &ll = orderMap.at(price);
            ll.erase(iter);
            if (ll.empty()) {
                orderMap.erase(price);
            }
        } else {
            auto &orderMap = sellOrdersBySymbol.at(order->symbol.id);
            auto price = order->price;
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

    std::optional<BuyOrdersAtPrice> buyOrders(SymbolId id) {
        return (buyOrdersBySymbol.find(id) == buyOrdersBySymbol.end()) ?
               std::nullopt : std::make_optional<BuyOrdersAtPrice>(buyOrdersBySymbol.at(id));
    }

    std::optional<SellOrdersAtPrice> sellOrders(SymbolId id) {
        return (sellOrdersBySymbol.find(id) == sellOrdersBySymbol.end()) ?
               std::nullopt : std::make_optional<SellOrdersAtPrice>(sellOrdersBySymbol.at(id));
    }

private:


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
    OrderEventListener<std::variant<platform::Ack, platform::TopOfBook, platform::Trades, Flush>> listner;
};
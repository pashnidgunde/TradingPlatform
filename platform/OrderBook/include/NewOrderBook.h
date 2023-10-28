#pragma once

#include <functional>
#include <array>
#include <optional>
#include <map>
#include <unordered_map>
#include <list>
#include <functional>
#include <variant>
#include <climits>

#include "WireFormat.h"
#include "OutgoingEvents.h"
#include "OrderEventListener.h"
#include <iostream>

template<typename T>
struct OrderStoreBase  {
    struct OrderIdentifierHasher {
        std::size_t operator()(const OrderIdentifier &oi) const {
            return ((std::hash<UserId>()(oi.userId))
                    ^ (std::hash<OrderId>()(oi.orderId) << 1));
        }
    };

    auto& get() { return static_cast<T*>(this)->orders; }
    auto& get(SymbolId symbol) { return static_cast<T*>(this)->orders[symbol]; }
    void flush() {
        for (auto &order: get()) {
            order.clear();
        }
        orderIdToNodeMap.clear();
    }
    auto insert(Order* order) {
        if (order->price == 0) {
            order->price = INT_MAX;
        }
        auto& ordersBySide = get(order->symbol.id);
        auto &ll = ordersBySide[order->price];
        auto it = ll.insert(ll.end(), order);
        orderIdToNodeMap[{order->oi.userId, order->oi.orderId}] = it;
    }

    bool remove(OrderIdentifier oi) {
        auto nodeIter = orderIdToNodeMap.find(oi);
        if (nodeIter == orderIdToNodeMap.end()) {
            return true;
        }
        auto llIter = nodeIter->second;
        auto orderPtr = *(nodeIter->second);
        auto &orderMap = this->get(orderPtr->symbol.id);
        if (orderMap.empty()) return false;

        auto price = orderPtr->price;
        if (orderMap.find(price) == orderMap.end()) {
            return false;
        }
        auto &ll = orderMap.at(price);
        ll.erase(llIter);
        if (ll.empty()) {
            orderMap.erase(price);
        }
        return true;
    }

    void reBalance(SymbolId symbol) {
        auto isFilled = [](const Order *order) {
            return order->qty == 0;
        };

        auto isMarketOrder = [](const Order *order) {
            return order->price == INT_MAX;
        };

        auto shouldDelete = [&](const Order *order) {
            return isFilled(order) || isMarketOrder(order);
        };

        auto& orders = get(symbol);

        for (auto it = orders.begin(); it != orders.end(); ++it) {
            auto &ll = it->second;
            for (auto llIter = ll.begin(); llIter != ll.end();) {
                if (shouldDelete(*llIter)) {
                    orderIdToNodeMap.erase({(*llIter)->oi});
                    llIter = ll.erase(llIter);
                } else {
                    break;
                }
            }
        }

        for (auto it = orders.cbegin(); it != orders.cend();) {
            if (it->second.empty()) {
                orders.erase(it++);
            } else {
                // short circuit as levels are ordered
                break;
            }
        }
    }

    std::unordered_map<OrderIdentifier, std::list<Order *>::iterator, OrderIdentifierHasher> orderIdToNodeMap;
};

template<char S>
struct OrderStore;

template<>
struct OrderStore<SIDE_BUY> : public OrderStoreBase<OrderStore<SIDE_BUY>>{
    using PriceLevels = std::map<Price, std::list<Order *>, std::greater<>>;
    std::array<PriceLevels, 1024> orders;
};

template<>
struct OrderStore<SIDE_SELL> : public OrderStoreBase<OrderStore<SIDE_SELL>>{
    using PriceLevels = std::map<Price, std::list<Order *>, std::less<>>;
    std::array<PriceLevels , 1024> orders;
};

template<typename O, typename T>
struct TTopOfBooksRAII {

    [[nodiscard]] const Order *topNow() const {
        if (!priceLevels.empty()) {
            auto &ll = priceLevels.cbegin()->second;
            if (!ll.empty()) {
                return ll.front();
            }
        }
        return nullptr;
    }

    bool changed() {
        return topBefore != topAfter;
    }

    TTopOfBooksRAII(O& observer, const T& priceLevels) :
        observer(observer),
        priceLevels(priceLevels) {
        topBefore = topNow();
    }

    ~TTopOfBooksRAII() {
        topAfter = topNow();
        if(changed()) {
            if constexpr(std::is_same<T,OrderStore<SIDE_BUY>>::value) {
                observer.onEvent(platform::TopOfBook<SIDE_BUY>(topAfter));
            }
            else {
                observer.onEvent(platform::TopOfBook<SIDE_SELL>(topAfter));
            }
        }
    }

    O &observer;
    const T& priceLevels;
    const Order *topBefore{nullptr};
    const Order *topAfter{nullptr};
};

template<typename O>
struct TopOfBooksRAII {
    TopOfBooksRAII(O &observer, OrderStore<SIDE_BUY>::PriceLevels& buyLevels, OrderStore<SIDE_SELL>::PriceLevels sellLevels) :
            bt(observer,buyLevels),
            st(observer,sellLevels) {
    }

    TTopOfBooksRAII<O, OrderStore<SIDE_BUY>::PriceLevels> bt;
    TTopOfBooksRAII<O, OrderStore<SIDE_SELL>::PriceLevels> st;
};


template<typename O>
class OrderBook {
public:

    explicit OrderBook(O &observer) :
        _observer(observer) {
    }

    void addOrder(Order *order) {
        _observer.onEvent(platform::Ack{order->oi});
        TopOfBooksRAII<O> t(_observer, buyStore.get(order->symbol.id), sellStore.get(order->symbol.id));

        if (order->side == SIDE_BUY) {
            buyStore.insert(order);
        } else if (order->side == SIDE_SELL) {
            sellStore.insert(order);
        } else {
            throw std::runtime_error("Unsupported side");
        }

        try_cross(order->symbol.id);
    }

    void flush() {
        buyStore.flush();
        sellStore.flush();
        _observer.onEvent(Flush{});
    }

    void cancel(const OrderIdentifier &oi) {
        if (!buyStore.remove(oi)) {
            sellStore.remove(oi);
        }
    }

    auto &buyOrders(SymbolId symbol) {
        return buyStore.get(symbol);
    }

    auto &sellOrders(SymbolId symbol) {
        return sellStore.get(symbol);
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
    }

    platform::Trades try_cross(const SymbolId &symbol) {
        bool hasBuys = !buyStore.get(symbol).empty();
        bool hasSells = !sellStore.get(symbol).empty();

        platform::Trades trades;
        if (hasBuys && hasSells) {
            auto canCross = [](const auto &buyPrice, const auto &sellPrice) {
                return buyPrice >= sellPrice;
            };

            for (auto &buysAtPriceLevel: buyStore.get(symbol)) {
                for (auto &sellsAtPriceLevel: sellStore.get(symbol)) {
                    if (!canCross(buysAtPriceLevel.first, sellsAtPriceLevel.first))
                        break;
                    auto matchPrice = std::min(buysAtPriceLevel.first, sellsAtPriceLevel.first);
                    cross(buysAtPriceLevel.second, sellsAtPriceLevel.second, matchPrice, trades);
                }
            }
        }

        buyStore.reBalance(symbol);
        sellStore.reBalance(symbol);

        _observer.onEvent(trades);
        return trades;
    }

    OrderStore<SIDE_BUY> buyStore;
    OrderStore<SIDE_SELL> sellStore;
    O &_observer;
};
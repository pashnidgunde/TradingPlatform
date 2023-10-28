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

using namespace std;

struct OrderIdentifierHasher {
    std::size_t operator()(const OrderIdentifier &oi) const {
        return ((std::hash<UserId>()(oi.userId))
                ^ (std::hash<OrderId>()(oi.orderId) << 1));
    }
};

using BuyOrdersAtPrice = std::map<Price, std::list<Order *>, std::greater<>>;
using SellOrdersAtPrice = std::map<Price, std::list<Order *>, std::less<>>;

template<typename O>
struct OrderBook;

template<typename T>
struct OrderStoreBase  {
    auto& get() { return static_cast<T*>(this)->orders; }
    auto& get(SymbolId symbol) { return static_cast<T*>(this)->orders[symbol]; }
    void flush() {
        for (auto &order: get()) {
            order.clear();
        }
    }
};

template<char S>
struct OrderStore;

template<>
struct OrderStore<SIDE_BUY> : public OrderStoreBase<OrderStore<SIDE_BUY>>{
    std::array<BuyOrdersAtPrice, 1024> orders;
};

template<>
struct OrderStore<SIDE_SELL> : public OrderStoreBase<OrderStore<SIDE_SELL>>{
    std::array<SellOrdersAtPrice , 1024> orders;
};

template<typename O, char S>
struct TTopOfBooksRAII {
    static constexpr char side = S;

    template<typename T>
    [[nodiscard]] const Order *top(const T &opl) const {
        if (!opl.empty()) {
            auto &ll = opl.cbegin()->second;
            if (!ll.empty()) {
                return ll.front();
            }
        }
        return nullptr;
    }

    bool changed() {
        return preTop != postTop;
    }

    const Order* getTop() const {
        if constexpr (S == SIDE_BUY) {
            return top(book.buyOrders(symbol));
        }
        return top(book.sellOrders(symbol));
    }

    TTopOfBooksRAII(OrderBook<O>& book, SymbolId symbol) :
        book(book),
        symbol(symbol) {
            preTop = getTop();
    }

    ~TTopOfBooksRAII() {
        postTop = getTop();
        if(changed()) {
            book.observer().onEvent(platform::TopOfBook<S>(postTop));
        }
    }

    OrderBook<O> &book;
    const SymbolId symbol{};
    const Order *preTop{nullptr};
    const Order *postTop{nullptr};
};

template<typename O>
struct TopOfBooksRAII {
    TopOfBooksRAII(OrderBook<O> &orderBook, SymbolId symbol) :
            bt(orderBook,symbol),
            st(orderBook,symbol) {
    }

    TTopOfBooksRAII<O,SIDE_BUY> bt;
    TTopOfBooksRAII<O,SIDE_SELL> st;
};

template<typename O>
class OrderBook {
public:

    explicit OrderBook(O &observer) :
        _observer(observer) {
    }

    template<typename T>
    void _add(T &existing, Order *incoming) {
        TopOfBooksRAII<O> t(*this, incoming->symbol.id);
        auto &ordersBySide = existing[incoming->symbol.id];
        auto &ll = ordersBySide[incoming->price];
        auto it = ll.insert(ll.end(), incoming);
        orderIdToNodeMap[{incoming->oi.userId, incoming->oi.orderId}] = it;
        try_cross(incoming->symbol.id);
    }

    void addOrder(Order *order) {
        if (order->price == 0) {
            order->price = INT_MAX;
        }
        _observer.onEvent(platform::Ack{order->oi});
        if (order->side == SIDE_BUY) {
            _add(buyStore.get(), order);
        } else if (order->side == SIDE_SELL) {
            _add(sellStore.get(), order);
        } else {
            throw std::runtime_error("Unsupported side");
        }
    }

    void flush() {
        buyStore.flush();
        sellStore.flush();

        orderIdToNodeMap.clear();
        _observer.onEvent(Flush{});
    }

    template<typename T>
    void _cancel(T &orders, const std::list<Order *>::iterator iter) {
        auto order = *iter;
        auto &orderMap = orders.at(order->symbol.id);
        auto price = order->price;
        auto &ll = orderMap.at(price);
        ll.erase(iter);
        if (ll.empty()) {
            orderMap.erase(price);
        }
    }

    void cancel(const OrderIdentifier &oi) {
        auto it = orderIdToNodeMap.find(oi);
        if (it == orderIdToNodeMap.end()) {
            return;
        }
        auto order = *it->second;
        if (order->side == SIDE_BUY) {
            _cancel(buyStore.get(), it->second);
        } else {
            _cancel(sellStore.get(), it->second);
        }
    }

    auto &buyOrders(SymbolId symbol) {
        return buyStore.get(symbol);
    }

    auto &sellOrders(SymbolId symbol) {
        return sellStore.get(symbol);
    }

    OrderStore<SIDE_BUY> buyStore;
    OrderStore<SIDE_SELL> sellStore;

    auto& observer() const { return _observer; }

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

    template<typename T>
    void removeFilledAndMarketOrders(T &ordersByPriceLevel) {
        auto isFilled = [](const Order *order) {
            return order->qty == 0;
        };

        auto isMarketOrder = [](const Order *order) {
            return order->price == INT_MAX;
        };

        auto shouldDelete = [&](const Order *order) {
            return isFilled(order) || isMarketOrder(order);
        };

        for (auto it = ordersByPriceLevel.begin(); it != ordersByPriceLevel.end(); ++it) {
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
    }

    template<typename T>
    void removeEmptyPriceLevel(T &ordersByPriceLevel) {
        for (auto it = ordersByPriceLevel.cbegin(); it != ordersByPriceLevel.cend();) {
            if (it->second.empty()) {
                ordersByPriceLevel.erase(it++);
            } else {
                // short circuit as levels are ordered
                break;
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

        if (hasBuys) {
            removeFilledAndMarketOrders(buyStore.get(symbol));
            removeEmptyPriceLevel(buyStore.get(symbol));
        }

        if (hasSells) {
            removeFilledAndMarketOrders(sellStore.get(symbol));
            removeEmptyPriceLevel(sellStore.get(symbol));
        }

        _observer.onEvent(trades);
        return trades;
    }


    
    std::unordered_map<OrderIdentifier, std::list<Order *>::iterator, OrderIdentifierHasher> orderIdToNodeMap;
    O &_observer;
};
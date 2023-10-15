#pragma once

#include <functional>
#include <array>
#include <optional>
#include <map>

#include "WireFormat.h"
#include "LinkedList.h"
#include "OutgoingEvents.h"

using namespace platform;

struct MatchFields {
    OrderIdentifier oi{};
    Qty qty{};

    bool operator==(const MatchFields &rhs) const {
        return oi == rhs.oi && qty == rhs.qty;
    }

    bool operator!=(const MatchFields &rhs) const {
        return !(rhs == *this);
    }
};


struct NewOrder {
    MatchFields of;
    SymbolId symbol{};
    Price price{};

    NewOrder(UserId u, OrderId o, Qty q, SymbolId s, Price p) :
            of{{u, o}, q}, symbol(s), price(p) {
    }

    bool operator==(const NewOrder &rhs) const {
        return of == rhs.of &&
               symbol == rhs.symbol &&
               price == rhs.price;
    }

    bool operator!=(const NewOrder &rhs) const {
        return !(rhs == *this);
    }
};



using FIFOOrderQueue = LinkedList<MatchFields>;

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

        FIFOOrderQueue::NodePtr insert(const NewOrder &order) {
            auto & ll = mutableOrdersAtPrice(order.price);
            auto node = ll.insert(order.of);
            return node;
        }

        [[nodiscard]] size_t size(Price price) const {
            return (ordersAtPrice(price).isEmpty()) ? 0 : ordersAtPrice(price).size();
        }

        [[nodiscard]] const auto& getAll() const {
            return static_cast<const T *>(this)->orders;
        }

        [[nodiscard]] auto& mutableAll() {
            return static_cast<T *>(this)->orders;
        }

        [[nodiscard]] std::optional<TopOfBook<SIDE>> top() const {
            if (isEmpty()) return std::nullopt;
            auto it = static_cast<const T *>(this)->orders.begin();
            return TopOfBook<SIDE>{it->first,it->second.front().qty};
        }
    };

    struct BuyOrders : public OrdersBySide<Side::BUY, BuyOrders> {
        std::map<Price, FIFOOrderQueue, std::greater<>> orders;
    };

    struct SellOrders : public OrdersBySide<Side::SELL, SellOrders> {
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

    auto addBuy(const NewOrder& order) {
        return book[order.symbol].buyOrders.insert(order);
    }

//    template<typename S, typename ...Args>
//    auto addBuy(S symbol, Args&& ...args) {
//        return book[symbol].buyOrders.insert(std::forward<NewOrder>(args...));
//    }

    auto addSell(const NewOrder& order) {
        return book[order.symbol].sellOrders.insert(order);
    }

    auto addSell(NewOrder&& order) {
        return book[order.symbol].sellOrders.insert(std::forward<NewOrder>(order));
    }

    auto buyTop(SymbolId symbol) const {
        return book[symbol].buyOrders.top();
    }

    auto sellTop(SymbolId symbol) const {
        return book[symbol].sellOrders.top();
    }

    auto &buyOrders(SymbolId symbol) const {
        return book[symbol].buyOrders;
    }

    auto &sellOrders(SymbolId symbol) const {
        return book[symbol].sellOrders;
    }

    auto &mutableBuyOrders(SymbolId symbol)  {
        return book[symbol].buyOrders.mutableAll();
    }

    auto &mutableSellOrders(SymbolId symbol) {
        return book[symbol].sellOrders.mutableAll();
    }

    void cross(FIFOOrderQueue& buys, FIFOOrderQueue& sells, const Price matchPrice, Trades& trades) {
        auto adjustOpenQty = [](MatchFields &of, int qty) { of.qty -= qty; };

        auto completelyFilled = [](MatchFields &of) { return of.qty == 0; };

        auto matchQty = 0;
        for (auto b = buys.begin(); b != buys.end(); ++b) {
            for (auto s = sells.begin(); s != sells.end(); ++s) {
                auto& buy = b->get();
                auto& sell = s->get();
                matchQty = std::min(buy.qty, sell.qty);

                trades.emplace_back(buy.oi,sell.oi, matchPrice, matchQty);
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

    platform::Trades tryCross(const SymbolId sId /*, char triggeredBy*/) {
        auto canCross = [](const auto &buyPrice, const auto &sellPrice) {
            return buyPrice >= sellPrice;
        };

        Trades trades;
        auto matchPrice = 0;
        for (auto & buyPair: mutableBuyOrders(sId)) {
            for (auto & sellPair: mutableSellOrders(sId)) {
                if (!canCross(buyPair.first, sellPair.first))
                    break;
                matchPrice = std::min(buyPair.first, sellPair.first);
                cross(buyPair.second, sellPair.second, matchPrice,trades);
            }
        }

        auto removeEmptyPriceLevels = [&](SymbolId sId) {
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

            removeEmptyPriceLevel(mutableBuyOrders(sId));
            removeEmptyPriceLevel(mutableSellOrders(sId));
        };
        removeEmptyPriceLevels(sId);

        return trades;
    }

    std::array<OrdersBySymbolId, SIZE> book;
};
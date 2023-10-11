#pragma once

#include <array>
#include <list>
#include <unordered_map>

#include "LinkedList.h"
#include "Order.h"
#include "OrderBookFields.h"
#include "OutgoingEvents.h"

namespace platform {

    template<size_t SIZE = 1024>
    struct OrderBook {
        using BuyOrders = LinkedList<OrderBookFields, std::greater<> >;
        using SellOrders = LinkedList<OrderBookFields, std::less<> >;
        using SymbolId = uint16_t;

        std::array<BuyOrders, SIZE> buys;
        std::array<SellOrders, SIZE> sells;

        using BuyNode = BuyOrders::value_type;
        using SellNode = SellOrders::value_type;
        using Trades = std::list<platform::Trade>;

        BuyOrders &buyOrders(const SymbolId sid) { return buys[sid]; }

        SellOrders &sellOrders(const SymbolId sid) {
            return sells[sid];
        }

        BuyNode addBuy(const SymbolId sid, const OrderBookFields &fields) {
            return buyOrders(sid).insert(fields);
        }

        SellNode addSell(const SymbolId sid, const OrderBookFields &fields) {
            return sellOrders(sid).insert(fields);
        }

        void removeFilled(const SymbolId sid) {
            auto removeCondition = [](const Node<OrderBookFields> *node) {
                return node->get().qty == 0;
            };
            buyOrders(sid).removeIf(removeCondition);
            sellOrders(sid).removeIf(removeCondition);
        }

        Trades tryCross(const SymbolId sid /*, char triggeredBy*/) {
            auto adjustQty = [](OrderBookFields &of, int qty) { of.qty -= qty; };

            auto filled = [](OrderBookFields &of) { return of.qty == 0; };

            auto canMatch = [](const auto &buy, const auto &sell) {
                return buy.price >= sell.price;
            };

            Trades trades;
            auto matchQty = 0;
            auto matchPrice = 0;
            for (auto buyIter: buyOrders(sid)) {
                for (auto sellIter: sellOrders(sid)) {
                    auto &buy = buyIter->get();
                    auto &sell = sellIter->get();
                    if (canMatch(buy, sell)) {
                        matchQty = std::min(buy.qty, sell.qty);
                        matchPrice = std::max(buy.price, sell.price);
                        trades.emplace_back(buy.oi, sell.oi, matchPrice, matchQty);
                        adjustQty(buy, matchQty);
                        adjustQty(sell, matchQty);
                        if (filled(buy)) {
                            break;
                        }
                    }
                }
            }
            this->removeFilled(sid);
            return trades;
        }
    };
}  // namespace platform

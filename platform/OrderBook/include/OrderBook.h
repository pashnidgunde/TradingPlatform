#pragma once
#include <list>
#include <unordered_map>

#include "LinkedList.h"
#include "Match.h"
#include "Order.h"
#include "OrderBookFields.h"

namespace platform {

struct OrderBook {
    using BuyOrders = LinkedList<OrderBookFields, std::greater<>>;
    using SellOrders = LinkedList<OrderBookFields, std::less<>>;
    std::unordered_map<std::string, std::pair<BuyOrders, SellOrders>> book;

    using BuyNode = BuyOrders::value_type;
    using SellNode = SellOrders::value_type;
    using Matches = std::list<platform::Match>;

    [[nodiscard]] bool isEmpty() const { return book.empty(); }

    BuyOrders& buyOrders(const std::string& symbol) { return book[symbol].first; }

    SellOrders& sellOrders(const std::string& symbol)
    {
        return book[symbol].second;
    }

    BuyNode addBuy(const std::string& symbol, OrderBookFields fields)
    {
        return buyOrders(symbol).insert(fields);
    }

    SellNode addSell(const std::string& symbol, const OrderBookFields& fields)
    {
        return sellOrders(symbol).insert(fields);
    }

    void removeFilled(const std::string& symbol)
    {
        auto removeCondition = [](const Node<OrderBookFields>* node) {
            return node->get().qty == 0;
        };
        buyOrders(symbol).removeIf(removeCondition);
        sellOrders(symbol).removeIf(removeCondition);
    }

    Matches tryMatch(const std::string& symbol /*, char triggeredBy*/)
    {
        auto adjustQty = [](OrderBookFields& of, int qty) { of.qty -= qty; };

        auto filled = [](OrderBookFields& of) { return of.qty == 0; };

        auto canMatch = [](const auto& buy, const auto& sell) {
            return buy.price >= sell.price;
        };

        Matches matches;
        auto matchQty = 0;
        auto matchPrice = 0;
        for (auto buyIter : buyOrders(symbol)) {
            for (auto sellIter : sellOrders(symbol)) {
                auto& buy = buyIter->get();
                auto& sell = sellIter->get();
                if (canMatch(buy, sell)) {
                    matchQty = std::min(buy.qty, sell.qty);
                    matchPrice = std::max(buy.price, sell.price);
                    matches.emplace_back(buy.oi, sell.oi, matchPrice, matchQty);
                    adjustQty(buy, matchQty);
                    adjustQty(sell, matchQty);
                    if (filled(buy)) {
                        break;
                    }
                }
            }
        }
        this->removeFilled(symbol);
        return matches;
    }
};
} // namespace platform

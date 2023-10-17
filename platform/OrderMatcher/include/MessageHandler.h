#pragma once

#include "WireFormat.h"
#include "../../OrderBook/include/OrderBook.h"
#include "Journal.h"
#include <variant>
#include "SymbolResolver.h"

struct OrderIdentifierHasher {
    std::size_t operator()(const OrderIdentifier &oi) const {
        return ((std::hash<UserId>()(oi.userId))
                ^ (std::hash<OrderId>()(oi.orderId) << 1));
    }
};

struct MessageHandler {
    void onIncoming(const Message &msg) {
        //validate
        if (!isValid(msg)) {
            std::cerr << "Failed to validate message" << std::endl;
            // NACK
            return;
        }

        //handler
        handle(msg);
    }

    bool isValid(const Message &msg) {
        auto isValidMsgType = [](char msgType, size_t msgLen) {
            if (msgType == MSGTYPE_NEW) {
                return msgLen == sizeof(Order);
            }
            else if (msgType == MSGTYPE_CANCEL) {
                return msgLen == sizeof(CancelOrder);
            }
            else if (msgType ==  MSGTYPE_FLUSH) {
                return msgLen == sizeof(Flush);
            }
            return false;
        };

        // TO DO : Compile time any_of
        return isValidMsgType(msg.type, msg.length);
    }

    auto addToSymbolBook(uint16_t symbolId, const Order *order) {
        FIFOOrderQueue::NodePtr node{nullptr};
        if (order->side == SIDE_BUY) {
            node = orderBook.addBuy({order->oi.userId, order->oi.orderId, order->qty, symbolId, order->price});
        } else if (order->side == SIDE_SELL) {
            node = orderBook.addSell({order->oi.userId, order->oi.orderId, order->qty, symbolId, order->price});
        } else {
            std::cerr << "Invalid side";
            // NACK
            return false;
        }

        // index for cancel lookup
        orderIdToNodeMap[{order->oi.userId, order->oi.orderId}] = node;

        // enqueue to print
        consoleJournal.log(Ack{order->oi});

        return true;
    }

    void handle(Message msg) {
        if (msg.type == MSGTYPE_NEW) {
            const auto *order = reinterpret_cast<const Order *>(msg.payload);

            auto symbolId = symbolResolver.resolve(order->symbol);

            if (!addToSymbolBook(symbolId, order)) return;

            auto trades = orderBook.tryCross(symbolId);
            if (!trades.empty()) {
                consoleJournal.log(trades);
            }

            auto newBuyTop = orderBook.buyTop(symbolId);
            if (newBuyTop.has_value()) {
                auto& value = newBuyTop.value();
                if ( value != latestBuyTop) {
                    consoleJournal.log(value);
                    latestBuyTop = value;
                }
            }

            auto newSellTop = orderBook.sellTop(symbolId);
            if (newSellTop.has_value()) {
                auto& value = newSellTop.value();
                if ( value != latestSellTop) {
                    consoleJournal.log(value);
                    latestSellTop = value;
                }
            }
        }
    }

    TopOfBook<SIDE_BUY> latestBuyTop;
    TopOfBook<SIDE_SELL> latestSellTop;
    SymbolResolver symbolResolver;
    OrderBook<1024> orderBook;
    Journal<std::variant<Ack, CancelAck, Trades, TopOfBook<SIDE_BUY>, TopOfBook<SIDE_SELL>>> consoleJournal;
    std::unordered_map<OrderIdentifier, FIFOOrderQueue::NodePtr, OrderIdentifierHasher> orderIdToNodeMap;
};

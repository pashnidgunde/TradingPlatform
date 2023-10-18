#pragma once

#include "WireFormat.h"
#include "../../OrderBook/include/NewOrderBook.h"
#include "SymbolResolver.h"
#include <iostream>

struct NewMessageHandler {
    void onIncoming(Message &msg) {
        //validate
        if (!isValid(msg)) {
            std::cerr << "Failed to validate message" << std::endl;
            // NACK
            return;
        }

        //handler
        handleIncoming(msg);
    }

    static bool isValid(const Message &msg) {
        auto isValidMsgType = [](char msgType, size_t msgLen) {
            if (msgType == MSGTYPE_NEW) {
                return msgLen == sizeof(Order);
            } else if (msgType == MSGTYPE_CANCEL) {
                return msgLen == sizeof(CancelOrder);
            } else if (msgType == MSGTYPE_FLUSH) {
                return msgLen == sizeof(Flush);
            }
            return false;
        };

        return isValidMsgType(msg.type, msg.length);
    }

    auto addToSymbolBook(const Order *order) {
        if (order->side == SIDE_BUY) {
            orderBook.addBuy(*order);
        } else if (order->side == SIDE_SELL) {
            orderBook.addSell(*order);
        } else {
            std::cerr << "Invalid side";
            // NACK
            return false;
        }
        return true;
    }

    template<MsgType>
    void handle(Message &) {}

    template<>
    void handle<MSGTYPE_NEW>(Message &msg) {
        auto *order = reinterpret_cast<Order *>(msg.payload);

        order->symbol.id = symbolResolver.resolve(order->symbol.name);

        if (!addToSymbolBook(order)) return;

        auto trades = orderBook.tryCross(order->symbol.id);

    }

    template<>
    void handle<MSGTYPE_CANCEL>(Message &msg) {
        const auto *c = reinterpret_cast<const CancelOrder *>(msg.payload);
        orderBook.cancel(c->oi);
    }

    template<>
    void handle<MSGTYPE_FLUSH>(Message & /*unused*/) {
        orderBook.flush();
    }

    void handleIncoming(Message &msg) {
        switch (msg.type) {
            case MSGTYPE_NEW :
                handle<MSGTYPE_NEW>(msg);
                break;

            case MSGTYPE_CANCEL :
                handle<MSGTYPE_CANCEL>(msg);
                break;

            case MSGTYPE_FLUSH :
                handle<MSGTYPE_FLUSH>(msg);
                break;

            default:
                std::cerr << "Invalid instruction";
        }
    }

    SymbolResolver symbolResolver;
    OrderBook<1024> orderBook;
};


#pragma once

#include "WireFormat.h"
#include "../../OrderBook/include/NewOrderBook.h"
#include "SymbolResolver.h"
#include <iostream>
#include "../../OrderBook/include/OrderEventListner.h"

struct NewMessageHandler {
    void onIncoming(const Message* msg) {
        //validate
        if (!isValid(msg)) {
            std::runtime_error("Failed to validate message");
            // NACK
            return;
        }

        //handler
        handleIncoming(const_cast<Message*>(msg));
    }

    static bool isValid(const Message *msg) {
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

        return isValidMsgType(msg->type, msg->length);
    }

    void addToSymbolBook(Order *order) {
        if (order->side == SIDE_BUY) {
            orderBook.addOrder(order);
        } else if (order->side == SIDE_SELL) {
            orderBook.addOrder(order);
        } else {
            std::runtime_error("Invalid side");
            // NACK
        }
    }

    void handleIncoming(Message *msg) {
        if (msg->type == MSGTYPE_NEW) {
            auto *order = reinterpret_cast<Order *>(msg->payload);
            order->symbol.id = symbolResolver.resolve(order->symbol.name);
            addToSymbolBook(order);
        } else if (msg->type == MSGTYPE_CANCEL) {
            const auto *c = reinterpret_cast<const CancelOrder *>(msg->payload);
            orderBook.cancel(c->oi);
        } else if (msg->type == MSGTYPE_FLUSH) {
            orderBook.flush();
        } else {
            std::runtime_error("Invalid MsgType");
        }
    }

    SymbolResolver symbolResolver;
    OrderBook orderBook;
};


#pragma once
#include "WireFormat.h"
#include "../../OrderBook/include/OrderBook.h"

template<char>
struct Handler{
};

template<>
struct Handler<MSGTYPE_NEW> {
    bool operator()(const Message& msg) {
        const Order *order = reinterpret_cast<const Order*>(msg.payload);
        switch(order->side) {
            case SIDE_BUY:
                break;
            case SIDE_SELL:
                break;
            default:
                std::cerr << "Invalid side";
                break;
        }
    }
};

struct MessageHandler {
    void onIncoming(const Message& msg) {
        //validate
        if (!isValid(msg)) {
            std::cerr << "Failed to validate message";
            // NACK
            return;
        }

        //handler
        handle(msg);
    }

    bool isValid(const Message& msg) {
        auto isValidMsgType = [](char msgType) {
            return (msgType == MSGTYPE_NEW ||
                    msgType == MSGTYPE_CANCEL ||
                    msgType == MSGTYPE_FLUSH);
        };
        if (msg.length != sizeof(Message)) return false;
        // TO DO : Compile time any_of
        return isValidMsgType(msg.type);
    }



    struct Common {
        OrderBook<1024> orderBook;
    };



    void handle(Message msg) {
        switch(msg.type) {
            case MSGTYPE_NEW :
                Handler<MSGTYPE_NEW>(msg);
                break;
        }

    }




};

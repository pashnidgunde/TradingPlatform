#pragma once

#include <string>
#include <vector>
#include "Tokenizer.h"
#include "WireFormat.h"
#include <string_view>
#include <memory>
#include <optional>

struct Encoder {
    static std::optional<Message> encode(const std::string &input) {
        std::vector<std::string> tokens = platform::util::Tokenizer::tokenize(input);
        Message msg;
        auto instruction = tokens[0][0];
        if (instruction == MSGTYPE_NEW) {
            msg.type = MSGTYPE_NEW;
            msg.length = sizeof(Order);
            auto order = encodeOrder(tokens);
            memcpy(msg.payload, &order, sizeof(Order));
            return msg;
        } else if (instruction == MSGTYPE_CANCEL) {
            msg.type = MSGTYPE_CANCEL;
            msg.length = sizeof(CancelOrder);
            auto cancel = encodedCancel(tokens);
            memcpy(msg.payload, &cancel, sizeof(CancelOrder));
            return msg;
        } else if (instruction == MSGTYPE_FLUSH) {
            msg.type = MSGTYPE_FLUSH;
            msg.length = sizeof(Flush);
            return msg;
        }
        return std::nullopt;
    }

    static Order encodeOrder(const std::vector<std::string> &tokens) {
        Order order;
        order.oi.userId = stoi(tokens[1]);
        memcpy(order.symbol.name, tokens[2].c_str(), std::min(sizeof(order.symbol.name), tokens[2].size()));
        order.price = stoi(tokens[3]);
        order.qty = stoi(tokens[4]);
        order.side = tokens[5][0];
        order.oi.orderId = stoi(tokens[6]);
        return order;
    }

    static CancelOrder encodedCancel(const std::vector<std::string> &tokens) {
        CancelOrder co;
        co.oi.userId = stoi(tokens[1]);
        co.oi.orderId = stoi(tokens[2]);
        return co;
    }
};

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
        if (tokens[0][0] == MSGTYPE_NEW) {
            msg.type = MSGTYPE_NEW;
            msg.length = sizeof(Order);
            auto order = encodeOrder(tokens);
            memcpy(msg.payload, &order, sizeof(Order));
            return msg;
        }
        return std::nullopt;
    }

    static Order encodeOrder(std::vector<std::string> tokens) {
        Order order;
        order.oi.userId = stoi(tokens[1]);
        memcpy(order.symbol, tokens[2].c_str(), std::min(sizeof(order.symbol), tokens[2].size()));
        order.price = stoi(tokens[3]);
        order.qty = stoi(tokens[4]);
        order.side = tokens[5][0];
        order.oi.orderId = stoi(tokens[6]);
        return order;
    }
};

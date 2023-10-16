#pragma once

#include <string>
#include <vector>
#include "Tokenizer.h"
#include "WireFormat.h"
#include <string_view>
#include <memory>
#include <optional>

struct Encoder {
    std::optional<Message> encode(const std::string &input) {
        std::vector<std::string> tokens = platform::util::Tokenizer::tokenize(input);
        Message msg;
        if (tokens[0][0] == MSGTYPE_NEW) {
            msg.type = MSGTYPE_NEW;
            msg.length = sizeof(Order);
            auto order = enocdeOrder(tokens);
            memcpy(msg.payload, &order, sizeof(Order));
            return msg;
        }
        return std::nullopt;
    }

    Order enocdeOrder(std::vector<std::string> tokens) {
        Order order;
        order.oi.userId = atoi(tokens[1].c_str());
        memcpy(order.symbol, tokens[2].c_str(), sizeof(tokens[2]));
        order.price = atoi(tokens[3].c_str());
        order.qty = atoi(tokens[4].c_str());
        order.side = tokens[5][0];
        order.oi.userId = atoi(tokens[6].c_str());
        return order;
    }
};

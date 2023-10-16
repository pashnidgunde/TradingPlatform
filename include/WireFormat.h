#pragma once

#include <cstdint>
#include <ostream>
#include <functional>

using Price = int;
using OrderId = int;
using UserId = int;
using Qty = int;
using SymbolId = int;
using Side = char;
using MsgType = char;


static constexpr char MSGTYPE_NEW = 'N';
static constexpr char MSGTYPE_CANCEL = 'C';
static constexpr char MSGTYPE_FLUSH = 'F';

static constexpr char SIDE_BUY = 'B';
static constexpr char SIDE_SELL = 'S';


struct Message {
    MsgType type{};
    uint8_t length{};
    char payload[64]{};
};

#pragma pack (push, 1)

struct OrderIdentifier {
    UserId userId = 0;
    OrderId orderId = 0;

    friend std::ostream &operator<<(std::ostream &os,
                                    const OrderIdentifier &identifier) {
        os << "userId: " << identifier.userId << " orderId: " << identifier.orderId;
        return os;
    }

    bool operator==(const OrderIdentifier &rhs) const {
        return userId == rhs.userId && orderId == rhs.orderId;
    }

    std::size_t operator()() const {
        return std::hash<int>{}(userId) << (std::hash<int>{}(orderId) << 1);
    }

    bool operator!=(const OrderIdentifier &rhs) const { return !(rhs == *this); }
};

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

struct Order {
    OrderIdentifier oi{};
    char symbol[32] = {0};
    Price price{};
    Qty qty{};
    Side side{};
};
static_assert(sizeof(Order) == 49);

struct CancelOrder {
    OrderIdentifier oi;
};
static_assert(sizeof(CancelOrder) == 8);

struct Flush {
};
static_assert(sizeof(Flush) == 1);

# pragma pack(pop)

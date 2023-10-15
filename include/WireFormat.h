#pragma once
#include <cstdint>
#include <ostream>

using Price = int;
using OrderId = int;
using UserId = int;
using Qty = int;
using SymbolId = int;
using SideType = char;
using MsgType = char;

struct MsgTypeVal {
    static constexpr char NEW = 'N';
    static constexpr char CANCEL = 'C';
    static constexpr char FLUSH = 'F';
};

struct Side {
    static constexpr char BUY = 'B';
    static constexpr char SELL = 'S';
};

struct Message {
    MsgType type{};
    uint8_t length{};
    char payload[64]{};
};

#pragma pack (push, 1)

struct OrderIdentifier {
    int userId = 0;
    int orderId = 0;

    friend std::ostream& operator<<(std::ostream& os,
                                    const OrderIdentifier& identifier) {
        os << "userId: " << identifier.userId << " orderId: " << identifier.orderId;
        return os;
    }

    bool operator==(const OrderIdentifier& rhs) const {
        return userId == rhs.userId && orderId == rhs.orderId;
    }

    bool operator!=(const OrderIdentifier& rhs) const { return !(rhs == *this); }
};

struct Order {
    OrderIdentifier oi;
    char symbol[32];
    Price price;
    Qty qty;
    SideType side;
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

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

    bool operator==(const OrderIdentifier &rhs) const {
        return userId == rhs.userId && orderId == rhs.orderId;
    }
};

struct Order {
    OrderIdentifier oi{};
    Qty qty{};

    union Symbol {
        Symbol() = default;

        Symbol(char *input) {
            memcpy(name, input, sizeof(name));
        }

        Symbol(SymbolId sid) : id(sid) {}

        char name[32] = {0};
        int id;
    };

    Symbol symbol{};
    Price price{};
    Side side{};

    Order() = default;

    Order(UserId u, OrderId o, Qty q, SymbolId sid, Price p) :
            oi{u, o}, qty(q), symbol(sid), price(p) {
    }

    Order(UserId u, OrderId o, Qty q, char *name, Price p) :
            oi{u, o}, qty(q), symbol(name), price(p) {
    }

    bool operator!=(const Order &rhs) const {
        return !(rhs == *this);
    }

    bool operator==(const Order &rhs) const {
        return oi.userId == rhs.oi.userId &&
               oi.orderId == rhs.oi.orderId;
    }
};

static_assert(sizeof(Order) == 49);

struct CancelOrder {
    OrderIdentifier oi;
};
static_assert(sizeof(CancelOrder) == 8);

struct Flush {
    friend std::ostream &operator<<(std::ostream &os, const Flush &) {
        os << std::endl;
        return os;
    }
};

static_assert(sizeof(Flush) == 1);

# pragma pack(pop)

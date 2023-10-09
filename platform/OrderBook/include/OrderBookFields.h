#pragma once

#include <ostream>
namespace platform {
struct OrderIdentifier {
    int userId = 0;
    int orderId = 0;

    friend std::ostream& operator<<(std::ostream& os,
        const OrderIdentifier& identifier)
    {
        os << "userId: " << identifier.userId << " orderId: " << identifier.orderId;
        return os;
    }

    bool operator==(const OrderIdentifier& rhs) const
    {
        return userId == rhs.userId && orderId == rhs.orderId;
    }

    bool operator!=(const OrderIdentifier& rhs) const { return !(rhs == *this); }

    OrderIdentifier() = default;
    OrderIdentifier(int uId, int oId)
        : userId(uId)
        , orderId(oId)
    {
    }
};

struct OrderBookFields {
    OrderIdentifier oi;
    int price = 0;
    int qty = 0;

    bool operator==(const OrderBookFields& rhs) const
    {
        return price == rhs.price;
    }

    friend std::ostream& operator<<(std::ostream& os,
        const OrderBookFields& fields)
    {
        os << "oi: " << fields.oi << " price: " << fields.price
           << " qty: " << fields.qty;
        return os;
    }

    OrderBookFields() = default;
    OrderBookFields(int uId, int oId, int p, int q)
        : oi(uId, oId)
        , price(p)
        , qty(q)
    {
    }

    bool operator<(const OrderBookFields& rhs) const { return price < rhs.price; }

    bool operator>(const OrderBookFields& rhs) const { return rhs < *this; }
};

static_assert(sizeof(OrderBookFields) == 16);
} // namespace platform

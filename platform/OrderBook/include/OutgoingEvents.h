#pragma once

#include <list>
#include <ostream>
#include "WireFormat.h"

namespace platform {
    struct Ack {
        static constexpr char value = 'A';
        OrderIdentifier sender;

        friend std::ostream &operator<<(std::ostream &os, const Ack &ack) {
            os << value << " ," << ack.sender.userId << " ," << ack.sender.orderId;
            return os;
        }
    };

    struct CancelAck {
        static constexpr char value = 'C';
        OrderIdentifier sender;

        friend std::ostream &operator<<(std::ostream &os, const CancelAck &ack) {
            os << value << " ," << ack.sender.userId << " ," << ack.sender.orderId;
            return os;
        }
    };

    struct Trade {
        static constexpr char value = 'T';
        OrderIdentifier buyer{};
        OrderIdentifier seller{};
        int price{};
        int qty{};

        Trade(const OrderIdentifier &buyer, const OrderIdentifier &seller, int price, int qty) :
                buyer(buyer),
                seller(seller),
                price(price), qty(qty) {}

        friend std::ostream &operator<<(std::ostream &os, const Trade &trade) {
            os << value << " ," << trade.buyer.userId << " ," << trade.buyer.orderId
               << " ," << trade.seller.userId << " ," << trade.seller.orderId
               << " ," << trade.price
               << " ," << trade.qty;
            return os;
        }
    };

    using Trades = std::list<platform::Trade>;

    template<char SIDE>
    struct TopOfBook {
        static constexpr char value = 'B';
        Price price{};
        Qty qty{};
        static constexpr char side = SIDE;

        friend std::ostream &operator<<(std::ostream &os, const TopOfBook &book) {
            os << value << " ," << side << " ," << book.price << " ," << book.qty;
            return os;
        }

        bool operator==(const TopOfBook &rhs) const {
            return price == rhs.price &&
                   qty == rhs.qty;
        }

        bool operator!=(const TopOfBook &rhs) const {
            return !(rhs == *this);
        }
    };

}  // namespace platform

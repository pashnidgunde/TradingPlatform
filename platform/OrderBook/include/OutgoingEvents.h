#pragma once

#include <list>
#include <ostream>
#include "WireFormat.h"

namespace platform {
    struct Ack {
        static constexpr char value = 'A';
        OrderIdentifier sender;

        friend std::ostream &operator<<(std::ostream &os, const Ack &ack) {
            os << value << ", " << ack.sender.userId << ", " << ack.sender.orderId;
            return os;
        }
    };

    struct CancelAck {
        static constexpr char value = 'C';
        OrderIdentifier sender;

        friend std::ostream &operator<<(std::ostream &os, const CancelAck &ack) {
            os << value << ", " << ack.sender.userId << ", " << ack.sender.orderId;
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
            os << value << ", " << trade.buyer.userId << ", " << trade.buyer.orderId
               << ", " << trade.seller.userId << ", " << trade.seller.orderId
               << ", " << trade.price
               << ", " << trade.qty;
            return os;
        }
    };

    using Trades = std::list<platform::Trade>;

    template<char S=SIDE_BUY>
    struct TopOfBook {
        static constexpr char value = 'B';
        static constexpr char side = S;
        explicit TopOfBook(const Order* order) : top(order) {}

        const Order* top = nullptr;

        friend std::ostream &operator<<(std::ostream &os, const TopOfBook &book) {
            os << value << ", " << side << ", ";

            if (book.top) {
                os << book.top->price << ", " << book.top->qty;
            }
            else {
                os << '-' << ", " << '-';
            }
            return os;
        }
    };


}  // namespace platform

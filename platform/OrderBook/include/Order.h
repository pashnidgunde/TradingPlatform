#pragma once
#include <cstdint>
namespace platform {
    template<typename IdType, typename PriceType, typename QtyType>
    struct Order {
        IdType id{};
        PriceType price{};
        QtyType qty{};

        Order() = delete;

        Order(const Order& other) : id(other.id), price(other.price), qty(other.qty)
        {}

        explicit Order(IdType i, PriceType p, QtyType q):
            id(i),price(p),qty(q)
        {}

        bool operator<(const Order &rhs) const {
            return price < rhs.price;
        }

        bool operator==(const Order &rhs) const {
            return id == rhs.id &&
                   price == rhs.price &&
                   qty == rhs.qty;
        }

        bool operator!=(const Order &rhs) const {
            return !(rhs == *this);
        }

        bool operator>(const Order &rhs) const {
            return rhs < *this;
        }
    };
}

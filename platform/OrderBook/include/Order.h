#pragma once
#include <cstdint>
namespace platform {
    template<typename IdType, typename PriceType, typename QtyType>
    struct Order {
        IdType id;
        PriceType price;
        QtyType qty;
    };
}

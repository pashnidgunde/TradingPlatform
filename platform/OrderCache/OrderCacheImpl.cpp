#include "OrderCacheImpl.h"

// add order to the cache
void OrderCacheImpl::addOrder(Order order) {
    auto& orders_by_orderid = m_orders.get<OrderIdTag>();
    orders_by_orderid.insert(order);
}

// remove order with this unique order id from the cache
void OrderCacheImpl::cancelOrder(const std::string& orderId) {
    auto& orders_by_orderid = m_orders.get<OrderIdTag>();
    orders_by_orderid.erase(orderId);
}

// remove all orders in the cache for this user
void OrderCacheImpl::cancelOrdersForUser(const std::string& user) {
    auto& orders_by_userid = m_orders.get<UserIdTag>();
    orders_by_userid.erase(user);
}

// remove all orders in the cache for this security with qty >= minQty
void OrderCacheImpl::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) {

    auto satisfiesCancelCondition = [](unsigned int qty, unsigned int minQty) {
        return qty >= minQty;
    };

    std::unordered_set<std::string> orderIds;
    auto range = m_orders.get<SecurityIdTag>().equal_range(securityId);
    for (auto begin = range.first; begin != range.second; begin++) {
        if (satisfiesCancelCondition(begin->qty(), minQty)) {
            orderIds.insert(begin->orderId());
        }
    }

    auto& orders_by_orderId = m_orders.get<OrderIdTag>();
    for (const auto& orderId : orderIds) {
        orders_by_orderId.erase(orderId);
    }
}

// return the total qty that can match for the security 
unsigned int OrderCacheImpl::getMatchingSizeForSecurity(const std::string& securityId) {
    auto buy_range = m_orders.get<SecurityIdSideTag>().equal_range(std::make_tuple(securityId, SIDE_BUY));
    auto sell_range = m_orders.get<SecurityIdSideTag>().equal_range(std::make_tuple(securityId, SIDE_SELL));

    auto shouldFilter = [](const auto& buy, const auto& sell) {
        return buy.company() == sell.company();
    };

    auto sweeped = [](const auto& order) {
        return order.qty() == 0;
    };

    auto matchingQty = [&](const auto& buy, const auto& sell) {
        return shouldFilter(buy, sell) ? 0 : std::min(buy.qty(), sell.qty());
    };

    /*struct newQty {
        explicit newQty(unsigned int qty) : m_qty(qty) {}
        unsigned int m_qty = 0;
        void operator()(Order& o) {
            o.setQty(o.qty() - m_qty);
        }
    };*/

    auto adjustQty = [&](const auto& order, unsigned int matchedQty) {
        auto& orders_by_orderid = m_orders.get<OrderIdTag>();
        auto it = orders_by_orderid.find(order.orderId());
        const_cast<Order&>(order).setQty(order.qty() - matchedQty);
        //orders_by_orderid.modify(it, newQty(order.qty() - matchedQty));
    };

    unsigned int total_matched = 0;

    std::unordered_set<std::string> sweepedOrderIds;
    for (auto buyIter = buy_range.first; buyIter != buy_range.second; buyIter++) {
        auto& buy = *buyIter;
        for (auto sellIter = sell_range.first; sellIter != sell_range.second; sellIter++) {
            auto& sell = *sellIter;
            auto matchedQty = matchingQty(*buyIter, *sellIter);

            if (matchedQty == 0) {
                continue;
            }

            total_matched += matchedQty;
            adjustQty(buy, matchedQty);
            adjustQty(sell, matchedQty);

            if (sweeped(buy)) {
                sweepedOrderIds.insert(buy.orderId());
                break;
            }

            if (sweeped(sell)) {
                sweepedOrderIds.insert(sell.orderId());
            }
        }
    }

    auto& orders_by_orderid = m_orders.get<OrderIdTag>();
    for (const auto& sweepedOrder : sweepedOrderIds) {
        orders_by_orderid.erase(sweepedOrder);
    }

    return total_matched;
}

// return all orders in cache in a vector
std::vector<Order> OrderCacheImpl::getAllOrders() const {
    std::vector<Order> r;
    auto& idx = m_orders.get<OrderIdTag>();
    r.reserve(idx.size());
    for (auto begin = idx.begin(); begin != idx.end(); begin++) {
        r.emplace_back(*begin);
    }
    return r;
}
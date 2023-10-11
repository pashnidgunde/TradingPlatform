#pragma once

#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

#include <unordered_set>
#include "OrderCache.h"
#include "OrderCacheImpl.h"

#include <type_traits>

class OrderCacheImpl : public OrderCacheInterface {
public:
    // add order to the cache
    void addOrder(Order order) override;

    // remove order with this unique order id from the cache
    void cancelOrder(const std::string &orderId) override;

    // remove all orders in the cache for this user
    void cancelOrdersForUser(const std::string &user) override;

    // remove all orders in the cache for this security with qty >= minQty
    void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId,
                                            unsigned int minQty) override;

    // return the total qty that can match for the security id
    unsigned int getMatchingSizeForSecurity(
            const std::string &securityId) override;

    // return all orders in cache in a vector
    std::vector<Order> getAllOrders() const override;

private:
    struct OrderIdTag;
    struct UserIdTag;
    struct SecurityIdTag;
    struct SecurityIdSideTag;

    using OrderIdType = std::invoke_result_t<decltype(&Order::orderId), Order>;
    static_assert(std::is_same_v<OrderIdType, std::string>);

    using UserIdType = std::invoke_result_t<decltype(&Order::user), Order>;
    static_assert(std::is_same_v<UserIdType, std::string>);

    using SecurityIdType =
            std::invoke_result_t<decltype(&Order::securityId), Order>;
    static_assert(std::is_same_v<SecurityIdType, std::string>);

    using SideType = std::invoke_result_t<decltype(&Order::side), Order>;
    static_assert(std::is_same_v<SideType, std::string>);

    using CompanyType = std::invoke_result_t<decltype(&Order::company), Order>;
    static_assert(std::is_same_v<CompanyType, std::string>);

    inline static const SideType SIDE_BUY = "Buy";
    inline static const SideType SIDE_SELL = "Sell";

    using Orders = boost::multi_index_container<
            Order,
            boost::multi_index::indexed_by<
                    boost::multi_index::hashed_unique<
                            boost::multi_index::tag<OrderIdTag>,
                            boost::multi_index::
                            const_mem_fun<Order, OrderIdType, &Order::orderId> >,
                    boost::multi_index::hashed_non_unique<
                            boost::multi_index::tag<UserIdTag>,
                            boost::multi_index::
                            const_mem_fun<Order, UserIdType, &Order::user> >,
                    boost::multi_index::hashed_non_unique<
                            boost::multi_index::tag<SecurityIdTag>,
                            boost::multi_index::
                            const_mem_fun<Order, SecurityIdType, &Order::securityId> >,
                    boost::multi_index::hashed_non_unique<
                            boost::multi_index::tag<SecurityIdSideTag>,
                            boost::multi_index::composite_key<
                                    Order,
                                    boost::multi_index::
                                    const_mem_fun<Order, SecurityIdType, &Order::securityId>,
                                    boost::multi_index::
                                    const_mem_fun<Order, SideType, &Order::side> > > > >;

    Orders m_orders;
};
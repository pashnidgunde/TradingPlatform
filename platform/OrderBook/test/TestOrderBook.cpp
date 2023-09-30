#include "OrderBook.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace platform;

class TestOrderBook : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }

    using OrderType = platform::Order<uint64_t,double,uint32_t>;
    using OrderBookType = platform::OrderBook<OrderType>;
};

TEST_F(TestOrderBook, IsEmptyInitially) {
    auto orderBook = std::make_unique<OrderBookType>();
    EXPECT_TRUE(orderBook->isEmpty());

}

TEST_F(TestOrderBook, addOrder) {
//    Order o("Any", "", "", 0, "", "");
//    orderBook->addOrder(o);
//    auto orders = orderBook->getAllOrders();
//    EXPECT_EQ(orders.size(), 1);
//    auto& oa = orders[0];
//    EXPECT_EQ(oa, o);
}





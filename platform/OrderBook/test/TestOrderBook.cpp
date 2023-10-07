#include "OrderBook.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace platform;

class TestOrderBook : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(TestOrderBook, IsEmptyInitially) {
  auto orderBook = std::make_unique<OrderBook>();
  EXPECT_TRUE(orderBook->isEmpty());
  EXPECT_TRUE(orderBook->buyOrders("Any").isEmpty());
  EXPECT_TRUE(orderBook->sellOrders("Any").isEmpty());
}

TEST_F(TestOrderBook, testOrderConstructor) {
  OrderBook b;
  std::string symbol = "IBM";
  EXPECT_TRUE(b.buyOrders("IBM").isEmpty());
  EXPECT_TRUE(b.sellOrders("IBM").isEmpty());
  b.addBuy("IBM", OrderBookFields(1, 1, 10, 100));
  EXPECT_FALSE(b.buyOrders("IBM").isEmpty());
  EXPECT_TRUE(b.sellOrders("IBM").isEmpty());
  b.addSell("IBM", OrderBookFields(1, 1, 10, 100));
  EXPECT_FALSE(b.sellOrders("IBM").isEmpty());

  EXPECT_EQ(b.buyOrders("IBM").size(), 1);
  EXPECT_EQ(b.sellOrders("IBM").size(), 1);

  b.addSell("IBM", OrderBookFields(1, 1, 10, 100));
  b.addSell("IBM", OrderBookFields(1, 1, 10, 100));
  EXPECT_EQ(b.sellOrders("IBM").size(), 3);
}

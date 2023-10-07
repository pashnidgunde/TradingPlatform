#include "OrderBook.h"
#include <gtest/gtest.h>
#include <random>

#include <iostream>

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

TEST_F(TestOrderBook, testBuyOrdering) {

    OrderBook b;
    std::string symbol = "IBM";
    std::random_device seed;
    std::mt19937 gen{seed()};                     // seed the generator
    std::uniform_int_distribution<> dist{1, 100}; // set min and max
    std::vector<OrderBookFields> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        inputs.emplace_back(i, i, dist(gen), i);
    }

    for (const auto& input : inputs) {
        b.addBuy("IBM",input);
    }

    std::sort(inputs.begin(), inputs.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.price > rhs.price;
    });

    std::vector<OrderBookFields> actual;
    auto & buyOrders = b.buyOrders("IBM");
    actual.reserve(10);
    for (auto iter = buyOrders.begin(); iter != buyOrders.end(); ++iter) {
        actual.emplace_back(iter->get());
    }

    EXPECT_EQ(inputs, actual);
}

TEST_F(TestOrderBook, testSellOrdering) {

    OrderBook b;
    std::string symbol = "IBM";
    std::random_device seed;
    std::mt19937 gen{seed()};                     // seed the generator
    std::uniform_int_distribution<> dist{1, 100}; // set min and max
    std::vector<OrderBookFields> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        inputs.emplace_back(i, i, dist(gen), i);
    }

    for (const auto& input : inputs) {
        b.addSell("IBM",input);
    }

    std::sort(inputs.begin(), inputs.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.price < rhs.price;
    });

    std::vector<OrderBookFields> actual;
    auto& sells = b.sellOrders("IBM");
    EXPECT_EQ(sells.size(), 10);
    actual.reserve(10);
    for (auto iter = sells.begin(); iter != sells.end(); ++iter) {
        actual.emplace_back(iter->get());
    }

    EXPECT_EQ(inputs, actual);
}

TEST_F(TestOrderBook, testNoMatch) {

    OrderBook b;
    std::string symbol = "IBM";
    std::random_device seed;
    std::vector<OrderBookFields> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        b.addBuy("IBM",OrderBookFields(i, i, 10, 10));
    }
    b.addSell("IBM", OrderBookFields(1, 1, 11, 100));
    auto matches = b.tryMatch("IBM");

    EXPECT_EQ(matches.size(), 0);
}

TEST_F(TestOrderBook, testSellSweep) {

    OrderBook b;
    std::string symbol = "IBM";
    std::random_device seed;
    std::vector<OrderBookFields> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        b.addSell("IBM",OrderBookFields(i, i, 10, 10));
    }
    b.addBuy("IBM", OrderBookFields(1, 1, 11, 100));
    auto matches = b.tryMatch("IBM");

    EXPECT_EQ(matches.size(), 10);
}

TEST_F(TestOrderBook, testBuySweep) {

    OrderBook b;
    std::string symbol = "IBM";
    std::random_device seed;
    std::vector<OrderBookFields> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        b.addBuy("IBM",OrderBookFields(i, i, 10, 10));
    }
    b.addSell("IBM", OrderBookFields(1, 1,9 , 100));
    auto matches = b.tryMatch("IBM");

    EXPECT_EQ(matches.size(), 10);
}
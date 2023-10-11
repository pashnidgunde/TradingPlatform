#include <gtest/gtest.h>
#include <random>
#include "OrderBook.h"

#include <iostream>

using namespace platform;

class TestOrderBook : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestOrderBook, IsEmptyInitially) {
  auto orderBook = std::make_unique<OrderBook<>>();
  EXPECT_TRUE(orderBook->buyOrders(0).isEmpty());
  EXPECT_TRUE(orderBook->sellOrders(0).isEmpty());
}

TEST_F(TestOrderBook, testOrderConstructor) {
  OrderBook b;
  EXPECT_TRUE(b.buyOrders(0).isEmpty());
  EXPECT_TRUE(b.sellOrders(0).isEmpty());
  b.addBuy(0, OrderBookFields(1, 1, 10, 100));
  EXPECT_FALSE(b.buyOrders(0).isEmpty());
  EXPECT_TRUE(b.sellOrders(0).isEmpty());
  b.addSell(0, OrderBookFields(1, 1, 10, 100));
  EXPECT_FALSE(b.sellOrders(0).isEmpty());

  EXPECT_EQ(b.buyOrders(0).size(), 1);
  EXPECT_EQ(b.sellOrders(0).size(), 1);

  b.addSell(0, OrderBookFields(1, 1, 10, 100));
  b.addSell(0, OrderBookFields(1, 1, 10, 100));
  EXPECT_EQ(b.sellOrders(0).size(), 3);
}

TEST_F(TestOrderBook, testBuyOrdering) {
  OrderBook b;
  std::string symbol{0};
  std::random_device seed;
  std::mt19937 gen{seed()};                      // seed the generator
  std::uniform_int_distribution<> dist{1, 100};  // set min and max
  std::vector<OrderBookFields> inputs;
  inputs.reserve(10);
  for (int i = 0; i < 10; ++i) {
    inputs.emplace_back(i, i, dist(gen), i);
  }

  for (const auto& input : inputs) {
    b.addBuy(0, input);
  }

  std::sort(inputs.begin(), inputs.end(), [](const auto& lhs, const auto& rhs) {
    return lhs.price > rhs.price;
  });

  std::vector<OrderBookFields> actual;
  auto& buyOrders = b.buyOrders(0);
  actual.reserve(10);
  for (auto iter = buyOrders.begin(); iter != buyOrders.end(); ++iter) {
    actual.emplace_back(iter->get());
  }

  EXPECT_EQ(inputs, actual);
}

TEST_F(TestOrderBook, testSellOrdering) {
  OrderBook b;
  std::string symbol{0};
  std::random_device seed;
  std::mt19937 gen{seed()};                      // seed the generator
  std::uniform_int_distribution<> dist{1, 100};  // set min and max
  std::vector<OrderBookFields> inputs;
  inputs.reserve(10);
  for (int i = 0; i < 10; ++i) {
    inputs.emplace_back(i, i, dist(gen), i);
  }

  for (const auto& input : inputs) {
    b.addSell(0, input);
  }

  std::sort(inputs.begin(), inputs.end(), [](const auto& lhs, const auto& rhs) {
    return lhs.price < rhs.price;
  });

  std::vector<OrderBookFields> actual;
  auto& sells = b.sellOrders(0);
  EXPECT_EQ(sells.size(), 10);
  actual.reserve(10);
  for (auto iter = sells.begin(); iter != sells.end(); ++iter) {
    actual.emplace_back(iter->get());
  }

  EXPECT_EQ(inputs, actual);
}

TEST_F(TestOrderBook, testNoMatch) {
  OrderBook b;
  std::string symbol{0};
  std::vector<OrderBookFields> inputs;
  inputs.reserve(10);
  for (int i = 0; i < 10; ++i) {
    b.addBuy(0, OrderBookFields(i, i, 10, 10));
  }
  b.addSell(0, OrderBookFields(1, 1, 11, 100));
  auto matches = b.tryCross(0);

  EXPECT_EQ(matches.size(), 0);

  auto& sells = b.sellOrders(0);
  for (auto iter = sells.begin(); iter != sells.end(); ++iter) {
    EXPECT_EQ(iter->get().qty, 100);
  }

  auto& buys = b.buyOrders(0);
  for (auto iter = buys.begin(); iter != buys.end(); ++iter) {
    EXPECT_EQ(iter->get().qty, 10);
  }
}

TEST_F(TestOrderBook, testSellSweep) {
  OrderBook b;
  std::vector<OrderBookFields> inputs;
  inputs.reserve(10);
  for (int i = 0; i < 10; ++i) {
    b.addSell(0, OrderBookFields(i, i, 10, 10));
  }
  b.addBuy(0, OrderBookFields(1, 1, 11, 100));
  auto matches = b.tryCross(0);

  EXPECT_EQ(matches.size(), 10);

  auto& sells = b.sellOrders(0);
  for (auto iter = sells.begin(); iter != sells.end(); ++iter) {
    EXPECT_EQ(iter->get().qty, 0);
  }
  auto& buys = b.buyOrders(0);
  for (auto iter = buys.begin(); iter != buys.end(); ++iter) {
    EXPECT_EQ(iter->get().qty, 0);
  }
}

TEST_F(TestOrderBook, testBuySweep) {
  OrderBook b;
  std::string symbol{0};
  std::vector<OrderBookFields> inputs;
  inputs.reserve(10);
  for (int i = 0; i < 10; ++i) {
    b.addBuy(0, OrderBookFields(i, i, 10, 10));
  }
  b.addSell(0, OrderBookFields(1, 1, 9, 100));
  auto matches = b.tryCross(0);

  EXPECT_EQ(matches.size(), 10);
  auto& buys = b.buyOrders(0);
  for (auto iter = buys.begin(); iter != buys.end(); ++iter) {
    EXPECT_EQ(iter->get().qty, 0);
  }
  auto& sells = b.buyOrders(0);
  for (auto iter = sells.begin(); iter != sells.end(); ++iter) {
    EXPECT_EQ(iter->get().qty, 0);
  }
}

TEST_F(TestOrderBook, testRemove) {
  OrderBook b;
  std::vector<OrderBookFields> inputs;
  inputs.reserve(10);

  b.addBuy(0, OrderBookFields(1, 1, 11, 10));
  b.addBuy(0, OrderBookFields(1, 1, 12, 10));
  b.addBuy(0, OrderBookFields(1, 1, 13, 10));
  b.addBuy(0, OrderBookFields(1, 1, 14, 10));
  b.addBuy(0, OrderBookFields(1, 1, 15, 10));
  b.addBuy(0, OrderBookFields(1, 1, 16, 10));
  b.addBuy(0, OrderBookFields(1, 1, 17, 10));
  b.addBuy(0, OrderBookFields(1, 1, 18, 10));
  b.addBuy(0, OrderBookFields(1, 1, 19, 10));
  b.addBuy(0, OrderBookFields(1, 1, 20, 10));

  b.addSell(0, OrderBookFields(1, 1, 20, 10));
  auto matches = b.tryCross(0);
  EXPECT_EQ(matches.size(), 1);

  auto node = b.addSell(0, OrderBookFields(1, 1, 16, 55));
  matches = b.tryCross(0);
  EXPECT_EQ(matches.size(), 4);
  EXPECT_EQ(node->get().qty, 15);

  node = b.addSell(0, OrderBookFields(1, 1, 10, 500));
  matches = b.tryCross(0);
  EXPECT_EQ(matches.size(), 5);
  EXPECT_EQ(node->get().qty, 450);
  EXPECT_EQ(b.buyOrders(0).size(), 0);
  EXPECT_EQ(b.sellOrders(0).size(), 2);

  EXPECT_EQ(b.buyOrders(0).begin(), b.buyOrders(0).end());
  auto sellIter = b.sellOrders(0).begin();
  EXPECT_EQ(sellIter->get().qty, 450);
  sellIter++;
  EXPECT_EQ(sellIter->get().qty, 15);
}

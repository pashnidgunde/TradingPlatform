#include "OrderBook.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <random>

using namespace platform;

class TestLinkedListOrderedByGreater : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
  using LinkedList = platform::LinkedList<OrderBookFields, std::greater<>>;
};

TEST_F(TestLinkedListOrderedByGreater, IsEmptyInitially) {
  auto ll = std::make_unique<LinkedList>();
  EXPECT_TRUE(ll->isEmpty());
  EXPECT_EQ(ll->size(), 0);
  EXPECT_EQ(ll->rbegin(), ll->rend());
  EXPECT_EQ(*(ll->rend()), nullptr);
}

TEST_F(TestLinkedListOrderedByGreater, addOrder) {
  auto ll = std::make_unique<LinkedList>();
  EXPECT_NE(nullptr, ll->insert(OrderBookFields(1, 1, 10.0, 10)));
  EXPECT_FALSE(ll->isEmpty());
  EXPECT_EQ(ll->size(), 1);
}

TEST_F(TestLinkedListOrderedByGreater, testTwoOrders) {
  auto ll = std::make_unique<LinkedList>();
  std::vector<OrderBookFields> inputs{
          OrderBookFields(1, 1, 11, 10),
          OrderBookFields(2, 1, 10, 10),
  };

  for (const auto &input : inputs) {
      EXPECT_NE(nullptr, ll->insert(input));
  }

  EXPECT_EQ(ll->size(), 2);

  LinkedList::Iterator begin = ll->begin();
  LinkedList::Iterator end = ll->end();
  std::vector<OrderBookFields> actual;
  while (begin != end) {
    actual.emplace_back(begin->get());
    begin++;
  }

  std::sort(inputs.begin(), inputs.end(), [](const auto &lhs, const auto &rhs) {
    return lhs.price > rhs.price;
  });

  EXPECT_EQ(actual, inputs);
}

TEST_F(TestLinkedListOrderedByGreater, testSameValues) {
    auto ll = std::make_unique<LinkedList>();
    std::vector<OrderBookFields> inputs{
            OrderBookFields(1, 1, 11, 10),
            OrderBookFields(1, 2, 11, 10),
            OrderBookFields(1, 3, 11, 10),
            OrderBookFields(1, 4, 11, 10),
    };

    for (const auto &input : inputs) {
        EXPECT_NE(nullptr, ll->insert(input));
    }

    EXPECT_EQ(ll->size(), 4);

    LinkedList::Iterator begin = ll->begin();
    LinkedList::Iterator end = ll->end();
    std::vector<OrderBookFields> actual;
    while (begin != end) {
        actual.emplace_back(begin->get());
        begin++;
    }

    EXPECT_EQ(actual, inputs);
}

TEST_F(TestLinkedListOrderedByGreater, testMultiple) {

  std::random_device seed;
  std::mt19937 gen{seed()};                     // seed the generator
  std::uniform_int_distribution<> dist{1, 100}; // set min and max
  std::vector<OrderBookFields> inputs;
  inputs.reserve(10);
  for (int i = 0; i < 10; ++i) {
    inputs.emplace_back(i, i, dist(gen), i);
  }

  LinkedList ll;
  for (const auto &e : inputs) {
      EXPECT_NE(nullptr, ll.insert(e));
  }

    std::sort(inputs.begin(), inputs.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.price > rhs.price || ((rhs.price == lhs.price) && lhs.oi.orderId > rhs.oi.orderId);
    });

  std::vector<OrderBookFields> actual;
  actual.reserve(10);
  for (auto iter = ll.begin(); iter != ll.end(); ++iter) {
    actual.emplace_back(iter->get());
  }

  EXPECT_EQ(inputs, actual);
}

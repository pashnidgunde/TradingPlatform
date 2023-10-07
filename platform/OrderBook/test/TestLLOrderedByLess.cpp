#include "OrderBookFields.h"
#include "OrderBook.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <random>

using namespace platform;

class TestLinkedListOrderedByLess : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}

  using LinkedList = platform::LinkedList<OrderBookFields, std::less<>>;
};

TEST_F(TestLinkedListOrderedByLess, IsEmptyInitially) {
  auto ll = std::make_unique<LinkedList>();
  EXPECT_TRUE(ll->isEmpty());
  EXPECT_EQ(ll->size(), 0);
  EXPECT_EQ(ll->begin(), ll->end());
}

TEST_F(TestLinkedListOrderedByLess, addOrder) {
  auto ll = std::make_unique<LinkedList>();
  EXPECT_NE(nullptr, ll->insert(OrderBookFields(1, 1, 10, 10)));
  EXPECT_FALSE(ll->isEmpty());
  EXPECT_EQ(ll->size(), 1);
}

TEST_F(TestLinkedListOrderedByLess, testTwoOrders) {
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
    return lhs.price < rhs.price;
  });

  EXPECT_EQ(actual, inputs);
}

TEST_F(TestLinkedListOrderedByLess, testMultiple) {

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
    return lhs.price < rhs.price;
  });

  std::vector<OrderBookFields> actual;
  actual.reserve(10);
  for (auto iter : ll) {
    actual.emplace_back(iter->get());
  }
  EXPECT_EQ(inputs, actual);
}

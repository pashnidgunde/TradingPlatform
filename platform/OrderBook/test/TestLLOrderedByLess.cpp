#include "MatchingFields.h"
#include "OrderBook.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <random>

using namespace platform;

class TestLinkedListOrderedByLess : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}

  using LinkedList = platform::LinkedList<MatchingFields, std::less<>>;
};

TEST_F(TestLinkedListOrderedByLess, IsEmptyInitially) {
  auto ll = std::make_unique<LinkedList>();
  EXPECT_TRUE(ll->isEmpty());
  EXPECT_EQ(ll->size(), 0);
  EXPECT_EQ(ll->begin(), ll->end());
  EXPECT_EQ(ll->begin(), nullptr);
}

TEST_F(TestLinkedListOrderedByLess, addOrder) {
  auto ll = std::make_unique<LinkedList>();
  ll->insert(MatchingFields(1, 1, 10, 10));
  EXPECT_FALSE(ll->isEmpty());
  EXPECT_EQ(ll->size(), 1);
}

TEST_F(TestLinkedListOrderedByLess, testTwoOrders) {
  auto ll = std::make_unique<LinkedList>();
  std::vector<MatchingFields> inputs{
      MatchingFields(1, 1, 11, 10),
      MatchingFields(2, 1, 10, 10),
  };

  for (const auto &input : inputs) {
    ll->insert(input);
  }

  EXPECT_EQ(ll->size(), 2);

  LinkedList::Iterator begin = ll->begin();
  LinkedList::Iterator end = ll->end();
  std::vector<MatchingFields> actual;
  while (begin != end) {
    actual.emplace_back(*begin);
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
  std::vector<MatchingFields> inputs;
  for (int i = 0; i < 10; ++i) {
    inputs.emplace_back(MatchingFields(i, i, dist(gen), i));
  }

  LinkedList ll;
  for (const auto &e : inputs) {
    ll.insert(e);
  }

  std::sort(inputs.begin(), inputs.end(), [](const auto &lhs, const auto &rhs) {
    return lhs.price < rhs.price;
  });

  std::vector<MatchingFields> actual;
  for (auto iter = ll.begin(); iter != ll.end(); ++iter) {
    actual.emplace_back((*iter));
  }

  EXPECT_EQ(inputs, actual);
}

#include "OrderBook.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <random>

using namespace platform;

class TestLinkedListOrderedByGreater : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }

    using OrderType = platform::Order<uint64_t,double,uint32_t>;
    using LinkedList = platform::LinkedList<OrderType, std::greater<OrderType>>;
};

TEST_F(TestLinkedListOrderedByGreater, IsEmptyInitially) {
    auto ll = std::make_unique<LinkedList>();
    EXPECT_TRUE(ll->isEmpty());
    EXPECT_EQ(ll->size(), 0);
    EXPECT_EQ(ll->rbegin(), ll->rend());
    EXPECT_EQ(ll->rend(), nullptr);
}

TEST_F(TestLinkedListOrderedByGreater, addOrder) {
    auto ll = std::make_unique<LinkedList>();
    ll->insert(OrderType(1,10.0,10));
    EXPECT_FALSE(ll->isEmpty());
    EXPECT_EQ(ll->size(),1);
}

TEST_F(TestLinkedListOrderedByGreater, testTwoOrders) {
    auto ll = std::make_unique<LinkedList>();
    std::vector<OrderType> inputs {
            OrderType(1,11,10),
            OrderType(2,10,10),
    };

    for (const auto& input : inputs) {
        ll->insert(input);
    }

    EXPECT_EQ(ll->size(), 2);

    LinkedList::Iterator begin = ll->begin();
    LinkedList::Iterator end = ll->end();
    std::vector<OrderType> actual;
    while(begin != end) {
        actual.emplace_back(*begin);
        begin++;
    }

    std::sort(inputs.begin(), inputs.end(),
              [](const auto& lhs, const auto& rhs) {
                  return lhs.price > rhs.price;
              });

    EXPECT_EQ(actual, inputs);
}


TEST_F(TestLinkedListOrderedByGreater, testMultiple) {

    std::random_device seed;
    std::mt19937 gen{seed()}; // seed the generator
    std::uniform_int_distribution<> dist{1, 100}; // set min and max
    std::vector<OrderType> inputs;
    for (int i=0; i< 10; ++i) {
        inputs.emplace_back(OrderType(i, dist(gen), i));
    }

    LinkedList ll;
    for (const auto& e : inputs) {
        ll.insert(e);
    }

    std::sort(inputs.begin(), inputs.end(),
              [](const auto& lhs, const auto& rhs) {
                  return lhs.price > rhs.price;
              });

    std::vector<OrderType> actual;
    for (auto iter= ll.begin(); iter != ll.end(); ++iter) {
        actual.emplace_back((*iter));
    }

    EXPECT_EQ(inputs, actual);
}

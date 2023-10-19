#include <gtest/gtest.h>
#include <random>
#include "NewOrderBook.h"

using namespace platform;

class TestNewOrderBook : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(TestNewOrderBook, IsisEmptyInitially) {
    OrderBook orderBook;
    EXPECT_TRUE(orderBook.isEmpty());
}

TEST_F(TestNewOrderBook, testOrderConstructor) {
    OrderBook b;
    constexpr int SYMBOL_IBM = 1;
    b.addOrder(Order(1, 1, 'B', SYMBOL_IBM, 100, 10));
    EXPECT_TRUE(b.buyOrders(SYMBOL_IBM).has_value());
    EXPECT_FALSE(b.sellOrders(SYMBOL_IBM).has_value());
    b.addOrder(Order(1, 1, 'S', SYMBOL_IBM, 100, 11));
    EXPECT_TRUE(b.sellOrders(SYMBOL_IBM).has_value());

    EXPECT_EQ(b.buyOrders(SYMBOL_IBM).value().size(), 1);
    EXPECT_EQ(b.sellOrders(SYMBOL_IBM).value().size(), 1);

    b.addOrder(Order(1, 1, 'S', SYMBOL_IBM, 100, 12));
    b.addOrder(Order(1, 1, 'S', SYMBOL_IBM, 100, 13));
    EXPECT_EQ(b.sellOrders(SYMBOL_IBM).value().size(), 3);
}

TEST_F(TestNewOrderBook, testBuyOrdering) {
    OrderBook b;
    std::string symbol{0};
    std::random_device seed;
    std::mt19937 gen{seed()};                      // seed the generator
    std::uniform_int_distribution<> dist{1, 100};  // set min and max
    std::vector<Order> inputs;
    constexpr int SYMBOL_IBM = 1;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        inputs.emplace_back(i, i, 'B', SYMBOL_IBM, 1, dist(gen));
    }

    for (auto &input: inputs) {
        b.addOrder(input);
    }

    std::sort(inputs.begin(), inputs.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.price > rhs.price;
    });

    std::vector<Order> actual;
    const auto &buys = b.buyOrders(SYMBOL_IBM);
    EXPECT_TRUE(buys.has_value());
    actual.reserve(10);
    for (const auto &buy: buys.value()) {
        const auto &ll = buy.second;
        for (auto &order: ll) {
            actual.emplace_back(order);
        }
    }

    EXPECT_EQ(inputs, actual);
}

TEST_F(TestNewOrderBook, testSellOrdering) {
    OrderBook b;
    constexpr int SYMBOL_IBM = 1;
    std::string symbol{0};
    std::random_device seed;
    std::mt19937 gen{seed()};                      // seed the generator
    std::uniform_int_distribution<> dist{1, 100};  // set min and max
    std::vector<Order> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        inputs.emplace_back(i, i, 'S', SYMBOL_IBM, 1, dist(gen));
    }

    for (auto &input: inputs) {
        b.addOrder(input);
    }

    std::sort(inputs.begin(), inputs.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.price < rhs.price;
    });

    std::vector<Order> actual;
    const auto &sells = b.sellOrders(SYMBOL_IBM);
    EXPECT_TRUE(sells.has_value());
    actual.reserve(10);
    for (const auto &sell: sells.value()) {
        const auto &ll = sell.second;
        for (auto &order: ll) {
            actual.emplace_back(order);
        }
    }

    EXPECT_EQ(inputs, actual);
}

TEST_F(TestNewOrderBook, testNoMatch) {
    OrderBook b;
    constexpr int SYMBOL_IBM = 1;
    for (int i = 0; i < 10; ++i) {
        b.addOrder(Order(i, i, 'B', SYMBOL_IBM, 10, 90));
    }
    b.addOrder(Order(2, 1, 'S', SYMBOL_IBM, 100, 100));
    auto matches = b.tryCross(SYMBOL_IBM);
    EXPECT_TRUE(matches.empty());
}

TEST_F(TestNewOrderBook, testCross) {
    OrderBook b;
    constexpr int SYMBOL_IBM = 1;

    b.addOrder(Order(2, 1, 'S', SYMBOL_IBM, 10, 10));
    b.addOrder(Order(1, 1, 'B', SYMBOL_IBM, 10, 11));

    auto matches = b.tryCross(SYMBOL_IBM);

    EXPECT_EQ(matches.size(), 1);

    EXPECT_TRUE(b.sellOrders(SYMBOL_IBM)->empty());
    EXPECT_TRUE(b.buyOrders(SYMBOL_IBM)->empty());
}


TEST_F(TestNewOrderBook, testSellSweep) {
    OrderBook b;
    constexpr int SYMBOL_IBM = 1;
    std::vector<Order> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        b.addOrder(Order(2, i, 'S', SYMBOL_IBM, 10, 10));
    }
    b.addOrder(Order(1, 1, 'B', SYMBOL_IBM, 100, 11));
    auto matches = b.tryCross(SYMBOL_IBM);

    EXPECT_EQ(matches.size(), 10);

    EXPECT_TRUE(b.sellOrders(SYMBOL_IBM)->empty());
    EXPECT_TRUE(b.buyOrders(SYMBOL_IBM)->empty());
}

TEST_F(TestNewOrderBook, testBuySweep) {
    OrderBook b;
    constexpr int SYMBOL_IBM = 1;
    std::string symbol{0};
    std::vector<Order> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        b.addOrder(Order(2, i, 'B', SYMBOL_IBM, 10, 10));
    }
    b.addOrder(Order(2, 1, 'S', SYMBOL_IBM, 100, 10));
    auto matches = b.tryCross(1);

    EXPECT_EQ(matches.size(), 10);

    EXPECT_TRUE(b.sellOrders(SYMBOL_IBM)->empty());
    EXPECT_TRUE(b.buyOrders(SYMBOL_IBM)->empty());
}

TEST_F(TestNewOrderBook, testCancel) {
    OrderBook b;
    constexpr int SYMBOL_IBM = 1;
    b.addOrder(Order(2, 1, 'B', SYMBOL_IBM, 10, 10));
    EXPECT_EQ(b.buyOrders(SYMBOL_IBM)->size(), 1);
    b.cancel({2, 1});
    EXPECT_EQ(b.buyOrders(SYMBOL_IBM)->size(), 0);
}


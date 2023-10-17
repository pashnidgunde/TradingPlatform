#include <gtest/gtest.h>
#include <random>
#include "OrderBook.h"

using namespace platform;

class TestOrderBook : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(TestOrderBook, IsisEmptyInitially) {
    auto orderBook = std::make_unique<OrderBook<>>();
    EXPECT_TRUE(orderBook->isEmpty(0));
    EXPECT_FALSE(orderBook->buyTop(0).has_value());
    EXPECT_FALSE(orderBook->sellTop(0).has_value());
}

TEST_F(TestOrderBook, testOrderConstructor) {
    OrderBook b;
    b.addBuy(Order(1, 1, 100, 1, 10));
    EXPECT_FALSE(b.buyOrders(1).isEmpty());
    EXPECT_TRUE(b.sellOrders(0).isEmpty());
    b.addSell(Order(1, 1, 100, 1, 10));
    EXPECT_FALSE(b.sellOrders(1).isEmpty());

    EXPECT_EQ(b.buyOrders(1).size(10), 1);
    EXPECT_EQ(b.sellOrders(1).size(10), 1);

    b.addSell(Order(1, 1, 100, 1, 10));
    b.addSell(Order(1, 1, 100, 1, 10));
    EXPECT_EQ(b.sellOrders(1).size(10), 3);
}

TEST_F(TestOrderBook, testBuyOrdering) {
    OrderBook b;
    std::string symbol{0};
    std::random_device seed;
    std::mt19937 gen{seed()};                      // seed the generator
    std::uniform_int_distribution<> dist{1, 100};  // set min and max
    std::vector<Order> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        inputs.emplace_back(i, i, i, 1, dist(gen));
    }

    for (const auto &input: inputs) {
        b.addBuy(input);
    }

    std::sort(inputs.begin(), inputs.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.price > rhs.price;
    });

    std::vector<Order> actual;
    const auto &buys = b.buyOrders(1).getAll();
    actual.reserve(10);
    for (const auto &buy: buys) {
        const auto &ll = buy.second;
        for (auto it = ll.begin(); it != ll.end(); ++it) {
            auto &fields = it->get();
            actual.emplace_back(fields.oi.userId, fields.oi.orderId, fields.qty, 1, buy.first);
        }
    }

    EXPECT_EQ(inputs, actual);
}

TEST_F(TestOrderBook, testSellOrdering) {
    OrderBook b;
    std::string symbol{0};
    std::random_device seed;
    std::mt19937 gen{seed()};                      // seed the generator
    std::uniform_int_distribution<> dist{1, 100};  // set min and max
    std::vector<Order> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        inputs.emplace_back(i, i, i, 1, dist(gen));
    }

    for (auto &input: inputs) {
        b.addSell(input);
    }

    std::sort(inputs.begin(), inputs.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.price < rhs.price;
    });

    std::vector<Order> actual;
    const auto &sells = b.sellOrders(1).getAll();
    actual.reserve(10);
    for (const auto &sell: sells) {
        const auto &ll = sell.second;
        for (auto it = ll.begin(); it != ll.end(); ++it) {
            auto &fields = it->get();
            actual.emplace_back(fields.oi.userId, fields.oi.orderId, fields.qty, 1, sell.first);
        }
    }

    EXPECT_EQ(inputs, actual);
}

TEST_F(TestOrderBook, testNoMatch) {
    OrderBook b;
    std::string symbol{0};
    for (int i = 0; i < 10; ++i) {
        b.addBuy(Order(i, i, 10, 1, 90));
    }
    b.addSell(Order(1, 1, 1000, 1, 100));
    auto matches = b.tryCross(1);

    EXPECT_EQ(matches.size(), 0);
}

TEST_F(TestOrderBook, testSellSweep) {
    OrderBook b;
    std::vector<Order> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        b.addSell(Order(i, i, 10, 1, 10));
    }
    b.addBuy(Order(1, 1, 100, 1, 11));
    auto matches = b.tryCross(1);

    EXPECT_EQ(matches.size(), 10);

    auto &sells = b.sellOrders(1);
    EXPECT_TRUE(sells.isEmpty());
    auto &buys = b.buyOrders(1);
    EXPECT_TRUE(buys.isEmpty());
}

TEST_F(TestOrderBook, testBuySweep) {
    OrderBook b;
    std::string symbol{0};
    std::vector<Order> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        b.addBuy(Order(i, i, 10, 1, 10));
    }
    b.addSell(Order(1, 1, 100, 1, 9));
    auto matches = b.tryCross(1);

    EXPECT_EQ(matches.size(), 10);

    auto &sells = b.sellOrders(1);
    EXPECT_TRUE(sells.isEmpty());
    auto &buys = b.buyOrders(1);
    EXPECT_TRUE(buys.isEmpty());
}

TEST_F(TestOrderBook, testRemove) {
    OrderBook b;

    for (int i = 10; i < 20; i++)
        b.addBuy(Order(1, 1, 10, 1, i));

    b.addSell(Order(2, 1, 20, 1, 10));
    auto matches = b.tryCross(1);
    EXPECT_EQ(matches.size(), 2);

    auto &sells = b.sellOrders(1);
    EXPECT_TRUE(sells.isEmpty());
    auto &buys = b.buyOrders(1);
    EXPECT_FALSE(buys.isEmpty());
    EXPECT_TRUE(buys.top().has_value());
    EXPECT_EQ(buys.top().value(), (TopOfBook<SIDE_BUY>{17, 10}));

    auto node = b.addSell(Order(2, 1, 55, 1, 16));
    matches = b.tryCross(1);
    EXPECT_EQ(matches.size(), 2);
    EXPECT_EQ(node->get().qty, 35);
    EXPECT_EQ(buys.top().value(), (TopOfBook<SIDE_BUY>{15, 10}));

    node = b.addSell(Order(2, 1, 500, 1, 10));
    matches = b.tryCross(1);
    EXPECT_EQ(matches.size(), 6);
    EXPECT_EQ(node->get().qty, 440);
}

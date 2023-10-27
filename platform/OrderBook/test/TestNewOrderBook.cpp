#include <gtest/gtest.h>
#include <random>
#include "NewOrderBook.h"
#include <thread>
#include "TestObserver.h"

using namespace platform;

using Observer = TestObserver<std::variant<platform::Ack, platform::TopOfBook, platform::Trades, Flush>>;

class TestNewOrderBook : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(TestNewOrderBook, IsEmptyInitially) {
    Observer observer;
    OrderBook orderBook(observer);
    EXPECT_TRUE(observer.orderedEvents().empty());
}

TEST_F(TestNewOrderBook, testOrderConstructor) {
    Observer observer;
    OrderBook b(observer);
    constexpr int SYMBOL_IBM = 1;
    b.addOrder(new Order(1, 1, 'B', SYMBOL_IBM, 100, 10));
    EXPECT_FALSE(b.buyOrders(SYMBOL_IBM).empty());
    EXPECT_TRUE(b.sellOrders(SYMBOL_IBM).empty());
    b.addOrder(new Order(1, 1, 'S', SYMBOL_IBM, 100, 11));
    EXPECT_FALSE(b.sellOrders(SYMBOL_IBM).empty());

    EXPECT_EQ(b.buyOrders(SYMBOL_IBM).size(), 1);
    EXPECT_EQ(b.sellOrders(SYMBOL_IBM).size(), 1);

    b.addOrder(new Order(1, 1, 'S', SYMBOL_IBM, 100, 12));
    b.addOrder(new Order(1, 1, 'S', SYMBOL_IBM, 100, 13));
    EXPECT_EQ(b.sellOrders(SYMBOL_IBM).size(), 3);

    EXPECT_FALSE(observer.orderedEvents().empty());

}

TEST_F(TestNewOrderBook, testBuyOrdering) {
    Observer observer;
    OrderBook b(observer);
    std::random_device seed;
    std::mt19937 gen{seed()};                      // seed the generator
    std::uniform_int_distribution<> dist{1, 100};  // set min and max
    std::vector<Order> inputs;
    constexpr int SYMBOL_IBM = 1;
    inputs.reserve(10);
    for (int i = 1; i < 10; ++i) {
        inputs.emplace_back(i, i, 'B', SYMBOL_IBM, 1, dist(gen));
    }

    for (auto &input: inputs) {
        b.addOrder(new Order(input));
    }

    std::this_thread::sleep_for(1s);

    std::sort(inputs.begin(), inputs.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.price > rhs.price;
    });

    std::vector<Order> actual;
    const auto &buys = b.buyOrders(SYMBOL_IBM);
    EXPECT_FALSE(buys.empty());
    actual.reserve(10);
    for (const auto &buy: buys) {
        const auto &ll = buy.second;
        for (auto order: ll) {
            actual.emplace_back(*order);
        }
    }

    EXPECT_EQ(inputs, actual);
}

TEST_F(TestNewOrderBook, testSellOrdering) {
    Observer observer;
    OrderBook b(observer);
    constexpr int SYMBOL_IBM = 1;
    std::string symbol{0};
    std::random_device seed;
    std::mt19937 gen{seed()};                      // seed the generator
    std::uniform_int_distribution<> dist{1, 100};  // set min and max
    std::vector<Order> inputs;
    inputs.reserve(10);
    for (int i = 1; i < 10; ++i) {
        inputs.emplace_back(i, i, 'S', SYMBOL_IBM, 1, dist(gen));
    }

    for (auto &input: inputs) {
        b.addOrder(new Order(input));
    }

    std::sort(inputs.begin(), inputs.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.price < rhs.price;
    });

    std::vector<Order> actual;
    const auto &sells = b.sellOrders(SYMBOL_IBM);
    EXPECT_FALSE(sells.empty());
    actual.reserve(10);
    for (const auto &sell: sells) {
        const auto &ll = sell.second;
        for (auto &order: ll) {
            actual.emplace_back(*order);
        }
    }

    EXPECT_EQ(inputs, actual);
}

TEST_F(TestNewOrderBook, testNoMatch) {
    Observer observer;
    OrderBook b(observer);
    constexpr int SYMBOL_IBM = 1;
    for (int i = 0; i < 10; ++i) {
        b.addOrder(new Order(i, i, 'B', SYMBOL_IBM, 10, 90));
    }
    b.addOrder(new Order(2, 1, 'S', SYMBOL_IBM, 100, 100));

    EXPECT_EQ(b.buyOrders(SYMBOL_IBM).size(), 1);
    EXPECT_EQ(b.sellOrders(SYMBOL_IBM).size(), 1);
}

TEST_F(TestNewOrderBook, testCross) {
    Observer observer;
    OrderBook b(observer);
    constexpr int SYMBOL_IBM = 1;

    b.addOrder(new Order(2, 1, 'S', SYMBOL_IBM, 10, 10));
    b.addOrder(new Order(1, 1, 'B', SYMBOL_IBM, 10, 11));

    EXPECT_TRUE(b.sellOrders(SYMBOL_IBM).empty());
    EXPECT_TRUE(b.buyOrders(SYMBOL_IBM).empty());
}


TEST_F(TestNewOrderBook, testSellSweep) {
    Observer observer;
    OrderBook b(observer);
    constexpr int SYMBOL_IBM = 1;
    std::vector<Order> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        b.addOrder(new Order(2, i, 'S', SYMBOL_IBM, 10, 10));
    }
    b.addOrder(new Order(1, 1, 'B', SYMBOL_IBM, 100, 11));

    EXPECT_TRUE(b.sellOrders(SYMBOL_IBM).empty());
    EXPECT_TRUE(b.buyOrders(SYMBOL_IBM).empty());
}

TEST_F(TestNewOrderBook, testBuySweep) {
    Observer observer;
    OrderBook b(observer);
    constexpr int SYMBOL_IBM = 1;
    std::string symbol{0};
    std::vector<Order> inputs;
    inputs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        b.addOrder(new Order(2, i, 'B', SYMBOL_IBM, 10, 10));
    }
    b.addOrder(new Order(2, 1, 'S', SYMBOL_IBM, 100, 10));

    EXPECT_TRUE(b.sellOrders(SYMBOL_IBM).empty());
    EXPECT_TRUE(b.buyOrders(SYMBOL_IBM).empty());
}

TEST_F(TestNewOrderBook, testCancel) {
    Observer observer;
    OrderBook b(observer);
    constexpr int SYMBOL_IBM = 1;
    b.addOrder(new Order(2, 1, 'B', SYMBOL_IBM, 10, 10));
    EXPECT_EQ(b.buyOrders(SYMBOL_IBM).size(), 1);
    b.cancel({2, 1});
    EXPECT_EQ(b.buyOrders(SYMBOL_IBM).size(), 0);
}

TEST_F(TestNewOrderBook, testMarketOrder) {
    Observer observer;
    OrderBook b(observer);
    constexpr int SYMBOL_IBM = 1;
    b.addOrder(new Order(2, 1, 'B', SYMBOL_IBM, 10, 0));
    EXPECT_EQ(b.buyOrders(SYMBOL_IBM).size(), 0);
}

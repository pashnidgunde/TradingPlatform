#include "LinkedList.h"
#include <gtest/gtest.h>

using namespace platform;

class TestLinkListReverseIterator : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(TestLinkListReverseIterator, testReverseIterInitialState) {
    auto ll = LinkedList<int>{};
    LinkedList<int>::ReverseIterator rbegin = ll.rbegin();
    LinkedList<int>::ReverseIterator rend = ll.rend();
    ASSERT_EQ(rbegin, ll.rend());
    ASSERT_EQ(rend, ll.rend());
}

TEST_F(TestLinkListReverseIterator, testReverseIterator) {
    LinkedList<int> ll;
    ll.insert(1);
    LinkedList<int>::ReverseIterator rbegin = ll.rbegin();
    ASSERT_NE(rbegin, ll.rend());
    ASSERT_EQ(*rbegin, 1);
    ASSERT_EQ(rbegin->get(), 1);

    ASSERT_NE(rbegin++, ll.rend());
    ASSERT_EQ(rbegin, ll.rend());

    rbegin = ll.rbegin();
    auto other = rbegin;
    ASSERT_EQ(rbegin, other);

    ASSERT_EQ(++rbegin, ll.rend());
}

TEST_F(TestLinkListReverseIterator, testReverseIter2Elements) {
    LinkedList<int> ll;
    ll.insert(1);
    ll.insert(2);
    LinkedList<int>::ReverseIterator rbegin = ll.rbegin();
    ASSERT_NE(rbegin, ll.rend());
    ASSERT_EQ(*rbegin, 2);
    ASSERT_EQ(rbegin->get(), 2);

    ASSERT_NE(rbegin++, ll.rend());
    ASSERT_NE(rbegin, ll.rend());
    ASSERT_EQ(rbegin->get(), 1);

    rbegin = ll.rbegin();
    auto other = rbegin;
    ASSERT_EQ(rbegin, other);
    ASSERT_NE(rbegin, ll.rend());
    ++rbegin;
    ASSERT_EQ(rbegin->get(), 1);
}

TEST_F(TestLinkListReverseIterator, testInsertAtMiddle) {
    LinkedList<int> ll;
    ll.insert(1);
    ll.insert(3);
    ll.insert(2);
    LinkedList<int>::ReverseIterator rbegin = ll.rbegin();
    ASSERT_NE(rbegin, ll.rend());
    ASSERT_EQ(*rbegin, 3);
    ASSERT_EQ(*(++rbegin), 2);
    ASSERT_EQ(*(++rbegin), 1);
    ASSERT_EQ(++rbegin, ll.end());
}
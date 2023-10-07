#include "LinkedList.h"
#include <gtest/gtest.h>

using namespace platform;

class TestLinkListIterator : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(TestLinkListIterator, testInitialState) {

  auto ll = LinkedList<int>{};
  LinkedList<int>::Iterator begin = ll.begin();
  LinkedList<int>::Iterator end = ll.end();
  ASSERT_EQ(begin, ll.end());
  ASSERT_EQ(end, ll.end());
}

TEST_F(TestLinkListIterator, testForwardIterator) {
  LinkedList<int> ll;
  EXPECT_NE(nullptr, ll.insert(1));
  LinkedList<int>::Iterator begin = ll.begin();
  ASSERT_NE(begin, ll.end());
  ASSERT_EQ(begin->get(), 1);

  ASSERT_NE(begin++, ll.end());
  ASSERT_EQ(begin, ll.end());

  begin = ll.begin();
  auto other = begin;
  ASSERT_EQ(begin, other);

  ASSERT_EQ(++begin, ll.end());
}

TEST_F(TestLinkListIterator, testForwardIterator2Elements) {
  LinkedList<int> ll;
    EXPECT_NE(nullptr, ll.insert(1));
    EXPECT_NE(nullptr, ll.insert(2));
  LinkedList<int>::Iterator begin = ll.begin();
  ASSERT_NE(begin, ll.end());
  ASSERT_EQ(begin->get(), 1);

  ASSERT_NE(begin++, ll.end());
  ASSERT_NE(begin, ll.end());
  ASSERT_EQ(begin->get(), 2);

  begin = ll.begin();
  auto other = begin;
  ASSERT_EQ(begin, other);
  ASSERT_NE(begin, ll.end());
  ++begin;
  ASSERT_EQ(begin->get(), 2);
}

TEST_F(TestLinkListIterator, testInsertAtMiddle) {
  LinkedList<int> ll;
    EXPECT_NE(nullptr, ll.insert(1));
    EXPECT_NE(nullptr, ll.insert(3));
    EXPECT_NE(nullptr, ll.insert(2));
  LinkedList<int>::Iterator begin = ll.begin();
  ASSERT_NE(begin, ll.end());
  ASSERT_EQ(begin->get(), 1);
  ASSERT_EQ((++begin)->get(), 2);
  ASSERT_EQ((++begin)->get(), 3);
  ASSERT_EQ(++begin, ll.end());
}

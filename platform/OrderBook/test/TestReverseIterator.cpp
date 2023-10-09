#include <gtest/gtest.h>
#include "LinkedList.h"

using namespace platform;

class TestLinkListReverseIterator : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestLinkListReverseIterator, testReverseIterInitialState) {
  using IntLL = LinkedList<int, std::less<>>;
  auto ll = IntLL{};
  IntLL::ReverseIterator rbegin = ll.rbegin();
  IntLL::ReverseIterator rend = ll.rend();
  ASSERT_EQ(rbegin, ll.rend());
  ASSERT_EQ(rend, ll.rend());
}

TEST_F(TestLinkListReverseIterator, testReverseIterator) {
  using IntLL = LinkedList<int, std::less<>>;
  IntLL ll;
  EXPECT_NE(nullptr, ll.insert(1));
  IntLL::ReverseIterator rbegin = ll.rbegin();
  ASSERT_NE(rbegin, ll.rend());
  ASSERT_EQ(rbegin->get(), 1);

  ASSERT_NE(rbegin++, ll.rend());
  ASSERT_EQ(rbegin, ll.rend());

  rbegin = ll.rbegin();
  auto other = rbegin;
  ASSERT_EQ(rbegin, other);

  ASSERT_EQ(++rbegin, ll.rend());
}

TEST_F(TestLinkListReverseIterator, testReverseIter2Elements) {
  using IntLL = LinkedList<int, std::less<>>;
  IntLL ll;
  EXPECT_NE(nullptr, ll.insert(1));
  EXPECT_NE(nullptr, ll.insert(2));
  IntLL::ReverseIterator rbegin = ll.rbegin();
  ASSERT_NE(rbegin, ll.rend());
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
  using IntLL = LinkedList<int, std::less<>>;
  IntLL ll;
  EXPECT_NE(nullptr, ll.insert(1));
  EXPECT_NE(nullptr, ll.insert(3));
  EXPECT_NE(nullptr, ll.insert(2));
  IntLL::ReverseIterator rbegin = ll.rbegin();
  ASSERT_NE(rbegin, ll.rend());
  ASSERT_EQ(rbegin->get(), 3);
  ASSERT_EQ((++rbegin)->get(), 2);
  ASSERT_EQ((++rbegin)->get(), 1);
  ASSERT_EQ(++rbegin, ll.rend());
}

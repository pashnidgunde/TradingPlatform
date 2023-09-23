#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "FixTagTransformer.h"

class TestFixTagTransformer : public ::testing::Test {
    
protected:
    void SetUp() override {
        
    }

    void TearDown() override {
    }
   
    
};

TEST_F(TestFixTagTransformer, testRemoveTag) {
    Statement s("35=D;44=D;");
    
    bool result = s.apply("35=;");
    EXPECT_TRUE(result);
    EXPECT_EQ(s.appplied(), "44=D;");
    
    result = s.apply("44=;");
    EXPECT_TRUE(result && s.appplied().empty());

    result = s.apply("55=;");
    EXPECT_EQ(s.appplied(), "");
}

TEST_F(TestFixTagTransformer, testReplaceTag) {
    Statement s("35=D;44=D;");
    
    bool result = s.apply("35=X;");
    EXPECT_TRUE(result);
    EXPECT_EQ(s.appplied(), "35=X;44=D;");
    
    result = s.apply("44=ABCD;");
    EXPECT_EQ(s.appplied(), "35=X;44=ABCD;");
}

TEST_F(TestFixTagTransformer, addMissingTag) {
    Statement s("35=D;44=D;");

    bool result = s.apply("99=ABCD;");
    EXPECT_EQ(s.appplied(), "35=D;44=D;99=ABCD;");

    result = s.apply("66=;");
    EXPECT_EQ(s.appplied(), "35=D;44=D;99=ABCD;");

}

TEST_F(TestFixTagTransformer, AddReplaceRemove) {
    Statement s("35=D;44=D;");

    bool result = s.apply("99=Add_99;");
    EXPECT_EQ(s.appplied(), "35=D;44=D;99=Add_99;");

    result = s.apply("44=Replace_44;");
    EXPECT_EQ(s.appplied(), "35=D;44=Replace_44;99=Add_99;");

    result = s.apply("35=;");
    EXPECT_EQ(s.appplied(), "44=Replace_44;99=Add_99;");
}


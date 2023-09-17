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

TEST_F(TestFixTagTransformer, testValidStatement) {
    //auto statement = (+(int_)) >> '=' >> *alnum >> ';';
    std::vector<std::string> validStatements =
    {
        "35=D;"
        ,"35=;"             // no value specified == reset
        ,"35=100000;"
    };
    
    for (const auto& validStatement : validStatements) {
        EXPECT_TRUE(FixTagTransformer::transform(validStatement));
    }
}

TEST_F(TestFixTagTransformer, testInvalidStatements) {
    //auto statement = (+(int_)) >> '=' >> *alnum >> ';';
    std::vector<std::string> validStatements =

    {
        "35=D"             // no semicolon
        ,"=D;"             // no tag specified
        ,"="               // no tag or no value or semilicon
        ,"-35=1000;"       // negative tag
    };

    for (const auto& validStatement : validStatements) {
        EXPECT_FALSE(FixTagTransformer::transform(validStatement));
    }
}




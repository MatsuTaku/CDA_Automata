//
//  CommomPrefixSet_test.cpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/11/17.
//

#include "gtest/gtest.h"
#include "csd_automata/CommonPrefixSet.hpp"

using namespace csd_automata;

TEST(CommonPrefixSetTest, Sequential) {
    std::string text = "Test text of here!!!";
    CommonPrefixSet cps(text);
    
    cps.AppendPrefix(4, 1);
    cps.AppendPrefix(9, 2);
    cps.AppendPrefix(12, 3);
    cps.AppendPrefix(20, 4);
    
    EXPECT_EQ(cps[0], "Test");
    EXPECT_EQ(cps[1], "Test text");
    EXPECT_EQ(cps[2], "Test text of");
    EXPECT_EQ(cps[3], "Test text of here!!!");
}

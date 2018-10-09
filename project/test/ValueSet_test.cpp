//
//  ValueSet_test.cpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/10/09.
//

#include "gtest/gtest.h"
#include "csd_automata/ValueSet.hpp"

TEST(ValueSet, convert) {
    const size_t size = 0x10000;
    std::vector<size_t> values(size);
    for (auto &v : values) {
        v = rand() % 0x10000;
    }
    csd_automata::ValueSet vs(values);
    
    for (auto i = 0; i < size; i++) {
        EXPECT_EQ(vs[i + 1], values[i]);
    }
}

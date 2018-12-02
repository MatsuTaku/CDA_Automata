//
//  MultipleVector_test.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/07/09.
//

#include "gtest/gtest.h"
#include "csd_automata/MultipleVector.hpp"

using namespace csd_automata;

TEST(MultipleVectorTest, Convert) {
    const auto size = 0x100000;
    
    std::vector<uint64_t> next_src(size);
    for (auto i = 0; i < size; i++) {
        auto rndWidth = rand() % 64;
        next_src[i] = 1ULL << rndWidth;
    }
    
    std::vector<uint64_t> check_src(size);
    for (auto i = 0; i < size; i++) {
        auto rndWidth = rand() % 64;
        check_src[i] = 1ULL << rndWidth;
    }
    
    MultipleVector fva;
    std::vector<int> unit_sizes = {8, 8};
    fva.set_value_sizes(unit_sizes);
    fva.resize(size);
    
    for (auto i = 0; i < size; i++) {
        fva.set<0>(i, next_src[i]);
        fva.set<1>(i, check_src[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        auto next = fva.get<0>(i);
        EXPECT_EQ(next, next_src[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        auto check = fva.get<1>(i);
        EXPECT_EQ(check, check_src[i]);
    }
    
}

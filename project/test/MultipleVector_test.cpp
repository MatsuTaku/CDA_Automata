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
    
    std::vector<uint64_t> nextSrc(size);
    for (auto i = 0; i < size; i++) {
        auto rndWidth = rand() % 64;
        nextSrc[i] = 1ULL << rndWidth;
    }
    
    std::vector<uint64_t> checkSrc(size);
    for (auto i = 0; i < size; i++) {
        auto rndWidth = rand() % 64;
        checkSrc[i] = 1ULL << rndWidth;
    }
    
    MultipleVector fva;
    std::vector<int> unitSizes = {8, 8};
    fva.setValueSizes(unitSizes);
    fva.resize(size);
    
    for (auto i = 0; i < size; i++) {
        fva.set<0>(i, nextSrc[i]);
        fva.set<1>(i, checkSrc[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        auto next = fva.get<0>(i);
        EXPECT_EQ(next, nextSrc[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        auto check = fva.get<1>(i);
        EXPECT_EQ(check, checkSrc[i]);
    }
    
}

//
//  MultipleVector_test.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/07/09.
//

#include "gtest/gtest.h"
#include "csd_automata/MultipleVector.hpp"

TEST(MultipleVectorTest, Convert) {
    const auto size = 0x10000;
    
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
    
    csd_automata::MultipleVector fva;
    fva.setValueSize(0, 8);
    fva.setValueSize(1, 8);
    fva.resize(size);
    
    for (auto i = 0; i < size; i++) {
        fva.set<0, uint64_t>(i, nextSrc[i]);
        fva.set<1, uint64_t>(i, checkSrc[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        auto next = fva.get<0, uint64_t>(i);
        EXPECT_EQ(next, nextSrc[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        auto check = fva.get<1, uint64_t>(i);
        EXPECT_EQ(check, checkSrc[i]);
    }
    
}

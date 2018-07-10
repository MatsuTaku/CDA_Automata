//
//  NextCheck_test.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/07/09.
//

#include "gtest/gtest.h"
#include "array_fsa/NextCheck.hpp"

TEST(NextCheckTest, UseDac) {
    const auto size = 0x1000000;
    
    std::vector<uint64_t> nextSrc(size);
    for (auto i = 0; i < size; i++) {
        auto rndWidth = rand() % 64;
        nextSrc[i] = 1ULL << rndWidth;
    }
    
    std::vector<uint64_t> checkSrc(size);
    for (auto i = 0; i < size; i++) {
        auto rndWidth = rand() % 32;
        checkSrc[i] = 1ULL << rndWidth;
    }
    
    array_fsa::NextCheck<true, true> nc;
    nc.resize(size);
    
    std::vector<bool> isStrIds(size);
    
    for (auto i = 0; i < size; i++) {
        nc.setNext(i, nextSrc[i]);
        auto check = checkSrc[i];
        if (check >> 8 == 0) {
            isStrIds[i] = false;
            nc.setCheck(i, check);
        } else {
            isStrIds[i] = true;
            nc.setStringId(i, check);
        }
    }
    nc.buildBitArray();
    
    for (auto i = 0; i < size; i++) {
        EXPECT_EQ(nc.next(i), nextSrc[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        if (!isStrIds[i])
            EXPECT_EQ(nc.check(i), checkSrc[i]);
        else
            EXPECT_EQ(nc.stringId(i), checkSrc[i]);
    }
    
}

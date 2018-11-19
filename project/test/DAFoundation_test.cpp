//
//  NextCheck_test.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/07/09.
//

#include "gtest/gtest.h"
#include "csd_automata/DAFoundation.hpp"

using namespace csd_automata;

namespace {
    
    std::vector<uint64_t> randVector(size_t size) {
        std::vector<uint64_t> src(size);
        auto bitsWidth = sim_ds::calc::sizeFitsInBits(size);
        for (auto i = 0; i < size; i++) {
            auto rndWidth = rand() % bitsWidth;
            src[i] = 1ULL << rndWidth;
        }
        return src;
    }
    
}

TEST(DAFoundationTest, UseDac) {
    const auto size = 0xFFFFF;

    auto nextSrc = randVector(size);
    auto checkSrc = randVector(size);

    DAFoundation<true, true, true, false, false, false, false> fd(size);

    std::vector<bool> isStrIds(size);

    for (auto i = 0; i < size; i++) {
        fd.setNext(i, nextSrc[i]);
        auto check = checkSrc[i];
        isStrIds[i] = check >> 7 > 0;
        if (!isStrIds[i]) {
            fd.setCheck(i, check);
        } else {
            fd.setStringId(i, check);
        }
    }
    fd.build();

    for (auto i = 0; i < size; i++) {
        EXPECT_EQ(fd.next(i), nextSrc[i]);
    }

    for (auto i = 0; i < size; i++) {
        if (!isStrIds[i])
            EXPECT_EQ(fd.check(i), checkSrc[i]);
        else
            EXPECT_EQ(fd.stringId(i), checkSrc[i]);
    }

}

TEST(DAFoundationTest, LookupDict) {
    const auto size = 0xFFFFF;
    
    auto nextSrc = randVector(size);
    auto checkSrc = randVector(size);
    auto cwordsSrc = randVector(size);
    
    DAFoundation<false, true, false, true, true, true, false> fd(size);
    
    std::vector<bool> isStrIds(size);
    
    for (auto i = 0; i < size; i++) {
        fd.setNext(i, nextSrc[i]);
        auto check = checkSrc[i];
        isStrIds[i] = check >> 7 > 0;
        fd.setIsString(i, isStrIds[i]);
        if (!isStrIds[i]) {
            fd.setCheck(i, check);
        } else {
            fd.setStringId(i, check);
        }
        fd.setCumWords(i, cwordsSrc[i]);
    }
    fd.build();
    
    for (auto i = 0; i < size; i++) {
        EXPECT_EQ(fd.next(i), nextSrc[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        if (!isStrIds[i])
            EXPECT_EQ(fd.check(i), checkSrc[i]);
        else
            EXPECT_EQ(fd.stringId(i), checkSrc[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        EXPECT_EQ(fd.cumWords(i), cwordsSrc[i]);
    }
    
}

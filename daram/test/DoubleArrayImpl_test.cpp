//
//  NextCheck_test.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/07/09.
//

#include "gtest/gtest.h"
#include "csd_automata/DoubleArrayImpl.hpp"

using namespace csd_automata;

namespace {

std::vector<uint64_t> RandVector(size_t size) {
    std::vector<uint64_t> src(size);
    auto bitsWidth = sim_ds::calc::SizeFitsInBits(size);
    for (auto i = 0; i < size; i++) {
        auto rnd_width = rand() % bitsWidth;
        src[i] = 1ULL << rnd_width;
    }
    return src;
}

}

TEST(DAFoundationTest, UseDac) {
    const auto size = 0xFFFFF;

    auto next_src = RandVector(size);
    auto check_src = RandVector(size);

    DoubleArrayImpl<true, true, true, false, false, false, false, false, false, false> fd;
    fd.resize(size);

    std::vector<bool> is_str_ids(size);

    for (auto i = 0; i < size; i++) {
        fd.set_next(i, next_src[i]);
        auto check = check_src[i];
        is_str_ids[i] = check >> 7 > 0;
        if (!is_str_ids[i]) {
            fd.set_check(i, check);
        } else {
            fd.set_string_id(i, check);
        }
    }
    fd.Freeze();

    for (auto i = 0; i < size; i++) {
        EXPECT_EQ(fd.next(i), next_src[i]);
    }

    for (auto i = 0; i < size; i++) {
        if (!is_str_ids[i])
            EXPECT_EQ(fd.check(i), check_src[i]);
        else
            EXPECT_EQ(fd.string_id(i), check_src[i]);
    }

}

TEST(DAFoundationTest, LookupDict) {
    const auto size = 0xFFFFF;
    
    auto next_src = RandVector(size);
    auto check_src = RandVector(size);
    auto cwords_src = RandVector(size);

    DoubleArrayImpl<false, true, false, true, true, true, false, false, false, false> fd;
    fd.resize(size);
    
    std::vector<bool> is_str_ids(size);
    
    for (auto i = 0; i < size; i++) {
        fd.set_next(i, next_src[i]);
        auto check = check_src[i];
        is_str_ids[i] = check >> 7 > 0;
        fd.set_is_string(i, is_str_ids[i]);
        if (!is_str_ids[i]) {
            fd.set_check(i, check);
        } else {
            fd.set_string_id(i, check);
        }
        fd.set_cum_words(i, cwords_src[i]);
    }
    fd.Freeze();
    
    for (auto i = 0; i < size; i++) {
        EXPECT_EQ(fd.next(i), next_src[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        if (!is_str_ids[i])
            EXPECT_EQ(fd.check(i), check_src[i]);
        else
            EXPECT_EQ(fd.string_id(i), check_src[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        EXPECT_EQ(fd.cum_words(i), cwords_src[i]);
    }
    
}

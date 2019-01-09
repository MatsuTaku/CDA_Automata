//
// Created by Kampersanda on 2017/05/31.
//

#include <cassert>
#include <iostream>
#include <unistd.h>

#include "gtest/gtest.h"
#include "csd_automata/PlainFSABuilder.hpp"


using namespace csd_automata;
using string_set = std::vector<std::string>;

namespace {
    
    string_set sample_strs() {
        return {
            "defied",
            "defies",
            "defy",
            "defying",
            "denied",
            "denies",
            "deny",
            "denying",
            "trie",
            "tried",
            "tries",
            "try",
            "trying"
        };
    }
    
    PlainFSA getPlainFSAFromStrings(const string_set& strs) {
        PlainFSABuilder builder;
        for (auto& str : strs) {
            builder.add(str);
        }
        return builder.release();
    }
    
    void checkPlainFSAHasMember(PlainFSA& fsa, const string_set& strs) {
        for (auto& str : strs) {
            auto state = fsa.get_root_state();
            size_t trans = 0;
            
            for (char c : str) {
                trans = fsa.get_trans(state, static_cast<uint8_t>(c));
                EXPECT_TRUE(trans != 0);
                state = fsa.get_target_state(trans);
            }
            EXPECT_TRUE(fsa.is_final_trans(trans));
        }
    }
    
    void test(const string_set& strs) {
        PlainFSA fsa = getPlainFSAFromStrings(strs);
        
        checkPlainFSAHasMember(fsa, strs);
        
//        fsa.print_for_debug(std::cout);
    }
    
}

TEST(PlainFSATest, build) {
    char *cwd ;
    cwd=getcwd(NULL, 0);
    printf("current working directory:%s\n", cwd);
    free(cwd);
    
    auto strs = sample_strs();
    test(strs);
    
}


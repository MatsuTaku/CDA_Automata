//
// Created by Kampersanda on 2017/05/31.
//

#include <cassert>
#include <iostream>

#include "PlainFSABuilder.hpp"

using namespace array_fsa;
using namespace std;

namespace {
    
    vector<string> sample_strs() {
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
    
    PlainFSA getPlainFSAFromStrings(vector<string> strs) {
        PlainFSABuilder builder;
        for (auto& str : strs) {
            builder.add(str);
        }
        return builder.release();
    }
    
    void checkPlainFSAHasMember(PlainFSA& fsa, vector<string> strs) {
        for (auto& str : strs) {
            auto state = fsa.get_root_state();
            size_t trans = 0;
            
            for (char c : str) {
                trans = fsa.get_trans(state, static_cast<uint8_t>(c));
                assert(trans != 0);
                state = fsa.get_target_state(trans);
            }
            assert(fsa.is_final_trans(trans));
        }
    }
    
    void test(const vector<string> &strs) {
        PlainFSA fsa = getPlainFSAFromStrings(strs);
        
        checkPlainFSAHasMember(fsa, strs);
        
        fsa.print_for_debug(std::cout);
    }
    
}

int main() {
    auto strs = sample_strs();
    test(strs);
    
    return 0;
}


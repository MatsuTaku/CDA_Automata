//
//  ArrayFSATailBuilder.cpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#include "array_fsa/ArrayFSATailBuilder.hpp"

#include "array_fsa/PlainFSA.hpp"
#include "array_fsa/StringDict.hpp"
#include "array_fsa/StringTransFSA.hpp"

#include "sim_ds/Log.hpp"

using namespace array_fsa;

template <class T>
void ArrayFSATailBuilder::showInBox(T &fsa) {
    auto tab = "\t";
    auto start = 0;
    for (auto i = start; i < start + 0x100; i++) {
        std::cout << i << tab << is_final_(i) << tab << get_next_(i) << tab << get_check_(i) << tab << has_label(i) << std::endl;
        std::cout << i << tab << fsa.isFinal(i) << tab << fsa.next(i) << tab << fsa.check(i) << tab << fsa.isStringTrans(i) << std::endl;
        if (has_label(i)) {
            sim_ds::Log::showAsBinary(get_label_number(i), 4);
            sim_ds::Log::showAsBinary(fsa.stringId(i), 4);
        }
        std::cout << std::endl;
    }
}

// MARK: - private

void ArrayFSATailBuilder::build_(bool binarymode) {
    bytes_.reserve(static_cast<size_t>(orig_fsa_.get_num_trans() * 1.1 * kElemSize));
    
    expand_();
    freeze_state_(0);
    set_true_final_and_used_next_(0);
    
    arrange_(orig_fsa_.get_root_state(), 0);
    
    // reset
    state_map_ = std::unordered_map<size_t, size_t>();
}


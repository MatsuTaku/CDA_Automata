//
//  StringDictBuilder.cpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/07.
//

#include "StringDictBuilder.hpp"

#include "basic.hpp"
#include <string.h>
#include "PlainFSA.hpp"
#include "StringDict.hpp"

using namespace array_fsa;

StringDict StringDictBuilder::build(const PlainFSA& fsa) {
    StringDictBuilder builder(fsa);
    builder.makeDict();
    builder.flatStringArray();
    
    StringDict dict;
    dict.str_dicts_ = std::move(builder.str_dicts_);
    dict.str_dict_indexes_ = std::move(builder.str_dict_indexes_);
    dict.has_label_bits_ = builder.has_label_bits_;
    dict.label_bytes_ = std::move(builder.label_bytes_);
    dict.finish_flags_ = std::move(builder.finish_flags_);
    return dict;
}

void StringDictBuilder::makeDict() {
    str_dict_indexes_.resize(orig_fsa_.bytes_.size() / PlainFSA::kTransSize);
    has_label_bits_.resize(orig_fsa_.bytes_.size() / PlainFSA::kTransSize);
    labelArrange(orig_fsa_.get_root_state());
}

void StringDictBuilder::labelArrange(size_t state) {
    const auto first_trans = orig_fsa_.get_first_trans(state);
    
    if (first_trans == 0) {
        return;
    }
    
    { // Set next of offset to state's second if needed.
        auto it = state_map_.find(state);
        if (it != state_map_.end()) {
            // already visited state
            return;
        }
    }
    
    state_map_.insert(std::make_pair(state, 0));
    
    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
        auto labelTrans = trans;
        if (orig_fsa_.is_straight_state(labelTrans)) {
            auto index = labelTrans / PlainFSA::kTransSize;
            has_label_bits_[index] = true;
            appendStrDict();
            str_dict_indexes_[index] = cur_str_dict_index_;
            cur_str_dict().set(orig_fsa_.get_trans_symbol(trans));
            do {
                labelTrans = orig_fsa_.get_target_state(labelTrans);
                cur_str_dict().set(orig_fsa_.get_trans_symbol(labelTrans));
            } while (orig_fsa_.is_straight_state(labelTrans));
        }
        
        labelArrange(orig_fsa_.get_target_state(labelTrans));
    }
}

void StringDictBuilder::flatStringArray() {
    for (auto &dict : str_dicts_) {
        dict.place = label_bytes_.size();
        for (auto &&c : dict.label) {
            label_bytes_.push_back(c);
        }
        finish_flags_.resize(label_bytes_.size(), false);
        finish_flags_[label_bytes_.size() - 1] = true;
    }
    
    // TODO: test
    for (auto &dict : str_dicts_) {
        for (auto i = 0; i < dict.label.size(); i++) {
            char dictC = dict.label[i];
            char flatC = label_bytes_[dict.place + i];
            if (dictC != flatC) {
                std::cout << "Error flat string array" << std::endl;
            }
            if (i == dict.label.size() - 1) {
                if (!finish_flags_[dict.place + i]) {
                    std::cout << "Error flat string finish flag" << std::endl;
                }
            }
        }
    }
}

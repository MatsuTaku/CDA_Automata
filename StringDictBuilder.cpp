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
#include <unordered_map>

using namespace array_fsa;

StringDict StringDictBuilder::build(const PlainFSA& fsa) {
    StringDictBuilder builder(fsa);
    builder.makeDict();
    builder.sortDicts();
    builder.flatStringArray();
    
    StringDict dict;
    dict.str_dicts_ = std::move(builder.str_dicts_);
    dict.fsa_target_indexes_ = std::move(builder.fsa_target_indexes_);
    dict.has_label_bits_ = builder.has_label_bits_;
    dict.label_bytes_ = std::move(builder.label_bytes_);
    dict.start_flags_ = std::move(builder.start_flags_);
    dict.finish_flags_ = std::move(builder.finish_flags_);
    return dict;
}

void StringDictBuilder::makeDict() {
    fsa_target_ids_.resize(orig_fsa_.bytes_.size() / PlainFSA::kTransSize);
    fsa_target_indexes_.resize(orig_fsa_.bytes_.size() / PlainFSA::kTransSize);
    has_label_bits_.resize(orig_fsa_.bytes_.size() / PlainFSA::kTransSize);
    labelArrange(orig_fsa_.get_root_state());
    dict_tri_ = ArrayTri(); // Clear memory
}

void StringDictBuilder::labelArrange(size_t state) {
    const auto first_trans = orig_fsa_.get_first_trans(state);
    
    if (first_trans == 0) {
        return;
    }
    
    {
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
            appendStrDict();
            cur_str_dict().set(orig_fsa_.get_trans_symbol(labelTrans));
            do {
                labelTrans = orig_fsa_.get_target_state(labelTrans);
                cur_str_dict().set(orig_fsa_.get_trans_symbol(labelTrans));
            } while (orig_fsa_.is_straight_state(labelTrans));
            saveStrDict(index);
        }
        
        labelArrange(orig_fsa_.get_target_state(labelTrans));
    }
}

void StringDictBuilder::sortDicts() {
    // Sort as Descending order to time of access to stringDict.
    std::sort(str_dicts_.begin(), str_dicts_.end(), [](const StrDictData &lhs, const StrDictData &rhs) {
        return lhs.counter > rhs.counter;
    });
    
    std::vector<size_t> idMap;
    idMap.resize(str_dicts_.size());
    
    for (auto i = 0; i < str_dicts_.size(); i++) {
        auto &dict = str_dicts_[i];
        idMap[dict.id] = i;
    }
    
    for (auto i = 0; i < fsa_target_ids_.size(); i++) {
        if (!has_label_bits_[i]) {
            continue;
        }
        auto &id = fsa_target_ids_[i];
        fsa_target_indexes_[i] = idMap[id];
    }
    
    { // Show mapping of byte size
        size_t counts[4] = {0, 0, 0, 0};
        for (auto size = 0; size < 4; size++) {
            size_t block = 1 << (8 * (size + 1));
            auto max = std::min(block, str_dicts_.size());
            for (auto i = (1 << (8 * size)) - 1; i < max; i++) {
                counts[size] += str_dicts_[i].counter + 1;
            }
        }
        int map[4];
        auto size = 0;
        for (auto i = 0; i < 4; i++) {
            size += counts[i];
        }
        for (auto i = 0; i < 4; i++) {
            map[i] = float(counts[i]) / size * 100;
        }
        std::cout << "Label index per\t" << std::endl;
        for (auto i = 0; i < 4; i++) {
            std::cout << map[i]<< "\t" ;
        }
        std::cout << "%";
        std::cout << " in " << size << std::endl;
    }
}

void StringDictBuilder::flatStringArray() {
    auto count = 0;
    for (auto &dict : str_dicts_) {
        start_flags_.resize(label_bytes_.size() + 1);
        start_flags_[label_bytes_.size()] = true;
        dict.place = label_bytes_.size();
        for (auto &&c : dict.label) {
            label_bytes_.push_back(c);
        }
        finish_flags_.resize(label_bytes_.size(), false);
        finish_flags_[label_bytes_.size() - 1] = true;
        count++;
    }
}

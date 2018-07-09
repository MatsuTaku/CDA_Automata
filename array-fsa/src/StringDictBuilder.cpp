//
//  StringDictBuilder.cpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/07.
//

#include "array_fsa/StringDictBuilder.hpp"

#include "array_fsa/StringDict.hpp"
#include "sim_ds/Calc.hpp"

using namespace array_fsa;


// Recusive function
inline void StringDictBuilder::labelArrange(size_t state) {
    const auto first_trans = orig_fsa_.get_first_trans(state);
    
    if (first_trans == 0 || // last trans
        state_map_.find(state) != state_map_.end()) {// already visited state
        return;
    }
    
    state_map_.insert(std::make_pair(state, 0));
    
    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
        auto labelTrans = trans;
        if (orig_fsa_.is_straight_state(labelTrans)) {
            auto index = labelTrans / PlainFSA::kTransSize;
            appendStrDict();
            curStrDict().set(orig_fsa_.get_trans_symbol(labelTrans));
            do {
                labelTrans = orig_fsa_.get_target_state(labelTrans);
                curStrDict().set(orig_fsa_.get_trans_symbol(labelTrans));
            } while (orig_fsa_.is_straight_state(labelTrans));
            saveStrDict(index);
        }
        
        labelArrange(orig_fsa_.get_target_state(labelTrans));
    }
    
}

void StringDictBuilder::appendStrDict() {
    StrDictData dict;
    dict.id = str_dicts_.size();
    str_dicts_.push_back(dict);
    cur_str_dict_index_ = str_dicts_.size() - 1;
}

void StringDictBuilder::saveStrDict(size_t index) {
    auto &dict = curStrDict();
    has_label_bits_[index] = true;
    dict.node_id = index;
    dict.counter = 1;
}

void StringDictBuilder::makeDict() {
    fsa_target_indexes_.resize(orig_fsa_.get_num_elements());
    has_label_bits_.resize(orig_fsa_.get_num_elements());
    labelArrange(orig_fsa_.get_root_state());
}

void StringDictBuilder::setSharings(bool mergeSuffix) {
    std::sort(str_dicts_.begin(), str_dicts_.end(), [](StrDictData &lhs, StrDictData &rhs) {
        return std::lexicographical_compare(lhs.label.rbegin(), lhs.label.rend(), rhs.label.rbegin(), rhs.label.rend());
    });
    
    StrDictData *owner = new StrDictData;
    for (auto i = str_dicts_.size(); i > 0; --i) {
        auto &cur = str_dicts_[i - 1];
        if (cur.label.size() == 0) {
            std::cerr << "label is empty!" << std::endl;;
            continue;
        }
        auto match = 0;
        auto oLen = owner->label.length();
        auto cLen = cur.label.length();
        while ((oLen > match && cLen > match) &&
               owner->label[oLen - match - 1] == cur.label[cLen - match - 1]) {
            ++match;
        }
        if (match > 0 && match == oLen) {
            // sharing
            cur.isIncluded = true;
            cur.enabled = false;
            cur.owner = owner->id;
            owner->counter++;
        } else if (mergeSuffix && (match > 0 && match == cLen)) {
            // included
            cur.isIncluded = true;
            cur.owner = owner->id;
            owner->counter++;
        } else {
            owner = &cur;
        }
    }
    
}

void StringDictBuilder::setUpLabelArray() {
    std::sort(str_dicts_.begin(), str_dicts_.end(), [](StrDictData &lhs, StrDictData &rhs) {
        return lhs.isIncluded != rhs.isIncluded ? lhs.isIncluded < rhs.isIncluded :
        lhs.entropy() > rhs.entropy();
    });
    updateIdMap();
    auto count = 0;
    for (auto &dict : str_dicts_) {
        auto index = label_array_.size();
        if (dict.isIncluded) {
            auto ownerDict = getDictFromId(dict.owner);
            if (ownerDict.place == -1) {
                abort();
            }
            index = ownerDict.place + ownerDict.label.size() - dict.label.size();
        }
        dict.place = index;
        fsa_target_indexes_[dict.node_id] = count++;
        if (!dict.isIncluded)
            label_array_.addString(dict.label);
    }
}

// MARK: - Log

void StringDictBuilder::showMappingOfByteSize() {
    size_t counts[4] = {0, 0, 0, 0};
    for (auto &dict : str_dicts_) {
        if (dict.isIncluded)
            continue;
        auto size = sim_ds::Calc::sizeFitInBytes(dict.place);
        if (size == 0) continue;
        counts[size - 1] += dict.counter + 1;
    }
    auto size = 0;
    for (auto c : counts)
        size += c;
    float map[4];
    for (auto i = 0; i < 4; i++)
        map[i] = float(counts[i]) / size * 100;
    std::cout << "Label index per\t" << std::endl;
    for (auto i = 0; i < 4; i++) {
        std::cout << int(map[i]) << "\t| " ;
    }
    std::cout << "%";
    std::cout << " in " << size << std::endl;
}

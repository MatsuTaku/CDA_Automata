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
#include "Calc.hpp"

using namespace array_fsa;

StringDict StringDictBuilder::build(const PlainFSA& fsa) {
    StringDictBuilder builder(fsa);
    builder.makeDict();
    builder.mergeTail();
    builder.sortDicts();
    builder.flatStringArray();
    builder.decideTargetIndexes();
    
    builder.showMappingOfByteSize();
    
    StringDict dict;
    dict.str_dicts_ = std::move(builder.str_dicts_);
    dict.fsa_target_indexes_ = std::move(builder.fsa_target_indexes_);
    dict.has_label_bits_ = builder.has_label_bits_;
    dict.label_bytes_ = std::move(builder.label_bytes_);
    return dict;
}

void StringDictBuilder::makeDict() {
    fsa_target_ids_.resize(orig_fsa_.bytes_.size() / PlainFSA::kTransSize);
    fsa_target_indexes_.resize(orig_fsa_.bytes_.size() / PlainFSA::kTransSize);
    has_label_bits_.resize(orig_fsa_.bytes_.size() / PlainFSA::kTransSize);
    labelArrange(orig_fsa_.get_root_state());
}

void StringDictBuilder::labelArrange(size_t state) {
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

void StringDictBuilder::appendStrDict() {
    StrDictData dict;
    dict.id = str_dicts_.size();
    str_dicts_.push_back(dict);
    cur_str_dict_index_ = str_dicts_.size() - 1;
}

void StringDictBuilder::saveStrDict(size_t index) {
    has_label_bits_[index] = true;
    
    auto revL = reverseString(cur_str_dict().label);
    auto sameAs = dict_reverse_tri_.isMember(revL);
    if (sameAs != 0) {
        str_dicts_[sameAs].counter++;
        fsa_target_ids_[index] = str_dicts_[sameAs].id;
        str_dicts_.pop_back();
        return;
    }
    fsa_target_ids_[index] = cur_str_dict().id;
    
    dict_reverse_tri_.add(revL, cur_str_dict_index_);
}

void StringDictBuilder::mergeTail() {
    // To protect start of 1byte zone
    sortDicts();
    
    updateIdMap();
    
    auto numIncluded = 0;
    auto includedRange = 0;
    auto maxRange = 0;
    for (auto &dict : str_dicts_) {
        auto revL = reverseString(dict.label);
        maxRange += revL.size();
        if (maxRange - revL.size() < 0x100) {
            // Protect start of 1byte zone
            // TODO: simple
//            continue;
        }
        auto isIncluded = dict_reverse_tri_.isIncluded(revL);
        if (isIncluded) {
            numIncluded++;
            includedRange += revL.size();
            setIncludedOwner(dict);
        }
    }
    
    std::sort(str_dicts_.begin(), str_dicts_.end(), [](const StrDictData &lhs, const StrDictData &rhs) {
        return lhs.isIncluded < rhs.isIncluded;
    });
    
//    std::cout << "Label included per : " << float(numIncluded) / str_dicts_.size() << std::endl;
//    std::cout << "Label reductable per : " << float(includedRange) / float(maxRange) << std::endl;
}

void StringDictBuilder::setIncludedOwner(StrDictData &dict) {
    auto revL = reverseString(dict.label);
    auto ownerId = dict_reverse_tri_.getOwnerIdIn(revL);
    
    dict.isIncluded = true;
    auto &owner = getDictFromId(ownerId);
    dict.owner = owner.id;
    owner.counter += dict.counter + 1;
}

// Sort as Descending order to time of access to stringDict.
void StringDictBuilder::sortDicts() {
    std::sort(str_dicts_.begin(), str_dicts_.end(), [](const StrDictData &lhs, const StrDictData &rhs) {
        return lhs.isIncluded != rhs.isIncluded ?
        lhs.isIncluded < rhs.isIncluded :
        lhs.entropy() > rhs.entropy();
    });
}

void StringDictBuilder::flatStringArray() {
    updateIdMap();
    for (auto &dict : str_dicts_) {
        if (dict.isIncluded) {
            auto &owner = getDictFromId(dict.owner);
            dict.place = owner.place + owner.label.size() - dict.label.size();
            continue;
        }
        dict.place = label_bytes_.size();
        for (auto &&c : dict.label) {
            label_bytes_.push_back(c);
        }
        label_bytes_.push_back('\0');
    }
}

void StringDictBuilder::decideTargetIndexes() {
    updateIdMap();
    
    for (auto i = 0; i < fsa_target_ids_.size(); i++) {
        if (!has_label_bits_[i])
            continue;
        auto id = fsa_target_ids_[i];
        fsa_target_indexes_[i] = idMap[id];
    }
}

// MARK: - Log

void StringDictBuilder::showMappingOfByteSize() {
    size_t counts[4] = {0, 0, 0, 0};
    for (auto &dict : str_dicts_) {
        if (dict.isIncluded)
            continue;
        auto size = Calc::sizeFitInBytes(dict.place);
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

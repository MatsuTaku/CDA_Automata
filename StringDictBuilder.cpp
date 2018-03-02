//
//  StringDictBuilder.cpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/07.
//

#include "StringDictBuilder.hpp"

#include "basic.hpp"
#include <string.h>
#include "StringDict.hpp"
#include <unordered_map>
#include "Calc.hpp"

using namespace array_fsa;

void StringDictBuilder::makeDict() {
    fsa_target_ids_.resize(orig_fsa_.get_bytes_size());
    fsa_target_indexes_.resize(orig_fsa_.get_bytes_size());
    has_label_bits_.resize(orig_fsa_.get_bytes_size());
    labelArrange(orig_fsa_.get_root_state());
    updateIdMap();
}

void StringDictBuilder::mergeTail() {
    // To protect start of 1byte zone
//    sortDicts();
    
//    auto maxRange = 0;
    for (auto &dict : str_dicts_) {
        auto revL = reverseString(dict.label);
//        maxRange += revL.size();
//        if (maxRange - revL.size() < 0x100) {
//            // Protect start of 1byte zone
//            // TODO: simple
//            continue;
//        }
        auto ownerId = sorted_string_list_.getOwnerId(revL);
        if (ownerId > -1) {
            setIncludedOwner(&dict, ownerId);
        }
    }
}

void StringDictBuilder::setIncludedOwner(StrDictData* dict, size_t ownerId) {
    dict->isIncluded = true;
    dict->owner = ownerId;
    auto &owner = getDictFromId(ownerId);
    owner.counter += dict->counter + 1;
}

// Sort as Descending order to time of access to stringDict.
void StringDictBuilder::sortDicts() {
    std::sort(str_dicts_.begin(), str_dicts_.end(),
              [](const StrDictData &lhs, const StrDictData &rhs) {
        return lhs.isIncluded != rhs.isIncluded ?
        lhs.isIncluded < rhs.isIncluded :
        lhs.entropy() > rhs.entropy();
    });
    
    updateIdMap();
}

void StringDictBuilder::flatStringArray() {
    // To decide place in owner, after decided owner's place.
    std::sort(str_dicts_.begin(), str_dicts_.end(),
              [](const StrDictData &lhs, const StrDictData &rhs) {
        return lhs.isIncluded < rhs.isIncluded;
    });
    updateIdMap();
    
    for (auto &dict : str_dicts_) {
        if (dict.isIncluded) {
            const auto &owner = getDictFromId(dict.owner);
            dict.place = owner.place + owner.label.size() - dict.label.size();
            continue;
        }
        dict.place = label_array_.size();
        label_array_.addString(dict.label);
    }
}

void StringDictBuilder::decideTargetIndexes() {
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

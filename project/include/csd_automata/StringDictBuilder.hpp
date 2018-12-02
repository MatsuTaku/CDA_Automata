//
//  StringDictBuilder.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/07.
//

#ifndef StringDictBuilder_hpp
#define StringDictBuilder_hpp

#include <unordered_map>

#include "basic.hpp"
#include "SerializedStringsBuilder.hpp"

#include "StringDict.hpp"
#include "PlainFSA.hpp"

#include "Director.hpp"

namespace csd_automata {

class StringDictBuilder {
    
    const PlainFSA& orig_fsa_;
    
    std::vector<StrDictData> str_dicts_;
    size_t cur_str_dict_index_;
    std::vector<size_t> fsa_target_indexes_;
    std::vector<bool> has_label_bits_;
    SerializedStringsBuilder label_array_;
    std::unordered_map<size_t, size_t> state_map_;
    
    std::vector<size_t> id_map_;
    
public:
    static void Build(StringDict&, const PlainFSA& fsa, bool binaryMode, bool mergeSuffix);
    
    ~StringDictBuilder() = default;
    
    StringDictBuilder(const StringDictBuilder&) = delete;
    StringDictBuilder& operator=(const StringDictBuilder&) = delete;
    
    StringDictBuilder(StringDictBuilder&&) = default;
    StringDictBuilder& operator=(StringDictBuilder&&) = default;
    
private:
    explicit StringDictBuilder(const PlainFSA& fsa, bool binaryMode) : orig_fsa_(fsa), label_array_(binaryMode) {}
    
    std::string string_reverse_(std::string text) const {
        reverse(text.begin(), text.end());
        return text;
    }
    
    void UpdateIdMap_() {
        id_map_ = {};
        id_map_.resize(str_dicts_.size());
        for (size_t i = 0, size = str_dicts_.size(); i < size; i++)
            id_map_[str_dicts_[i].id] = i;
    }
    
    StrDictData& GetDictFromId_(size_t id) {
        return str_dicts_[id_map_[id]];
    }
    
    StrDictData& CurrentStrDict_() {
        return str_dicts_[cur_str_dict_index_];
    }
    
    // Recursive function
    void LabelArrange_(size_t state);
    
    void AppendStrDict_();
    void SaveStrDict_(size_t index);
    
    void MakeDict_();
    void SetSharings_(bool merge_suffix);
    void SetUpLabelArray_();
    
    void ShowMappingOfByteSize_();
    
};


// MARK: - Static build function

inline void StringDictBuilder::Build(StringDict& dict, const PlainFSA& fsa, bool binaryMode, bool mergeSuffix) {
    StringDictBuilder builder(fsa, binaryMode);
    std::cout << "------ StringDict build bench mark ------" << std::endl;
    
    auto inTime = director::MeasureProcessing([&]() {
        builder.MakeDict_();
    });
    std::cout << "makeDict: " << inTime << "ms" << std::endl;
    
    inTime = director::MeasureProcessing([&]() {
        builder.SetSharings_(mergeSuffix);
    });
    std::cout << "setSharings: " << inTime << "ms" << std::endl;
    
    inTime = director::MeasureProcessing([&]() {
        builder.SetUpLabelArray_();
    });
    std::cout << "setUpLabelArray: " << inTime << "ms" << std::endl;
    
//        builder.showMappingOfByteSize();
    
    dict.str_dicts_ = builder.str_dicts_;
    dict.fsa_target_indexes_ = builder.fsa_target_indexes_;
    dict.has_label_bits_ = builder.has_label_bits_;
    dict.label_array_ = std::move(builder.label_array_);
}


// MARK: - Member functions

// Recusive function
inline void StringDictBuilder::LabelArrange_(size_t state) {
    const auto first_trans = orig_fsa_.get_first_trans(state);
    
    if (first_trans == 0 || // last trans
        state_map_.find(state) != state_map_.end()) {// already visited state
        return;
    }
    
    state_map_.insert(std::make_pair(state, 0));
    
    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
        auto labelTrans = trans;
        if (orig_fsa_.is_straight_state(labelTrans)) {
            auto index = labelTrans / PlainFSA::kSizeTrans;
            AppendStrDict_();
            CurrentStrDict_().set(orig_fsa_.get_trans_symbol(labelTrans));
            do {
                labelTrans = orig_fsa_.get_target_state(labelTrans);
                CurrentStrDict_().set(orig_fsa_.get_trans_symbol(labelTrans));
            } while (orig_fsa_.is_straight_state(labelTrans));
            SaveStrDict_(index);
        }
        
        LabelArrange_(orig_fsa_.get_target_state(labelTrans));
    }
    
}

void StringDictBuilder::AppendStrDict_() {
    StrDictData dict;
    dict.id = str_dicts_.size();
    str_dicts_.push_back(dict);
    cur_str_dict_index_ = str_dicts_.size() - 1;
}

void StringDictBuilder::SaveStrDict_(size_t index) {
    auto &dict = CurrentStrDict_();
    has_label_bits_[index] = true;
    dict.node_id = index;
    dict.counter = 1;
}

void StringDictBuilder::MakeDict_() {
    fsa_target_indexes_.resize(orig_fsa_.get_num_elements());
    has_label_bits_.resize(orig_fsa_.get_num_elements());
    LabelArrange_(orig_fsa_.get_root_state());
}

void StringDictBuilder::SetSharings_(bool mergeSuffix) {
    std::sort(str_dicts_.begin(), str_dicts_.end(), [](StrDictData &lhs, StrDictData &rhs) {
        return std::lexicographical_compare(lhs.label.rbegin(), lhs.label.rend(), rhs.label.rbegin(), rhs.label.rend());
    });
    
    auto dummy = StrDictData();
    StrDictData *owner = &dummy;
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
            cur.is_included = true;
            cur.enabled = false;
            cur.owner = owner->id;
            owner->counter++;
        } else if (mergeSuffix && (match > 0 && match == cLen)) {
            // included
            cur.is_included = true;
            cur.owner = owner->id;
            owner->counter++;
        } else {
            owner = &cur;
        }
    }
    
}

void StringDictBuilder::SetUpLabelArray_() {
    std::sort(str_dicts_.begin(), str_dicts_.end(), [](StrDictData &lhs, StrDictData &rhs) {
        return lhs.is_included != rhs.is_included ? lhs.is_included < rhs.is_included :
        lhs.entropy() > rhs.entropy();
    });
    UpdateIdMap_();
    auto count = 0;
    for (auto &dict : str_dicts_) {
        auto index = label_array_.size();
        if (dict.is_included) {
            auto ownerDict = GetDictFromId_(dict.owner);
            if (ownerDict.place == -1) {
                abort();
            }
            index = ownerDict.place + ownerDict.label.size() - dict.label.size();
        }
        dict.place = index;
        fsa_target_indexes_[dict.node_id] = count++;
        if (!dict.is_included)
            label_array_.AddString(dict.label);
    }
}

// MARK: - Log

void StringDictBuilder::ShowMappingOfByteSize_() {
    size_t counts[4] = {0, 0, 0, 0};
    for (auto &dict : str_dicts_) {
        if (dict.is_included)
            continue;
        auto size = sim_ds::calc::SizeFitsInBytes(dict.place);
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

}

#endif /* StringDictBuilder_hpp */

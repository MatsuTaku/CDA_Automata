//
//  TailDictBuilder.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/07.
//

#ifndef TailDictBuilder_hpp
#define TailDictBuilder_hpp

#include <unordered_map>

#include "basic.hpp"
#include "SerializedStringsBuilder.hpp"

#include "TailDict.hpp"
#include "PlainFSA.hpp"

#include "Director.hpp"

namespace csd_automata {

class TailDictBuilder {
public:
    static void Build(TailDict&, const PlainFSA& fsa, bool binary_mode, bool merge_suffix, bool divide_front);
    
    TailDictBuilder() = default;
    ~TailDictBuilder() = default;
    
    TailDictBuilder(const TailDictBuilder&) = delete;
    TailDictBuilder& operator=(const TailDictBuilder&) = delete;
    
    TailDictBuilder(TailDictBuilder&&) = default;
    TailDictBuilder& operator=(TailDictBuilder&&) = default;
    
private:
    const PlainFSA& orig_fsa_;
    
    std::vector<StrDictData> str_dicts_;
    size_t cur_str_dict_index_;
    std::vector<size_t> fsa_target_indexes_;
    std::vector<bool> has_label_bits_;
    SerializedStringsBuilder label_array_;
    std::unordered_map<size_t, size_t> state_map_;
    
    std::vector<size_t> id_map_;
    
    explicit TailDictBuilder(const PlainFSA& fsa, bool binary_mode) : orig_fsa_(fsa), label_array_(binary_mode) {}
    
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
    
    StrDictData& current_dict_() {
        return str_dicts_[cur_str_dict_index_];
    }
    
    // Recursive function
    void LabelArrange_(size_t state);
    
    void NewFace_();
    void SaveStrDict_(size_t index);
    
    void MakeDict_();
    void SetSharing_(bool merge_suffix);
    void SetUpLabelArray_(bool divide_front);
    
    void ShowMappingOfByteSize_();
    
};


// MARK: - Static build function

inline void TailDictBuilder::Build(TailDict& dict, const PlainFSA& fsa, bool binary_mode, bool merge_suffix, bool divide_front) {
    TailDictBuilder builder(fsa, binary_mode);
    std::cout << "------ TailDict build bench mark ------" << std::endl;
    
    auto inTime = director::MeasureProcessing([&builder]() {
        builder.MakeDict_();
    });
    std::cout << "makeDict: " << inTime << "ms" << std::endl;
    
    inTime = director::MeasureProcessing([&builder, merge_suffix]() {
        builder.SetSharing_(merge_suffix);
    });
    std::cout << "setSharings: " << inTime << "ms" << std::endl;
    
    inTime = director::MeasureProcessing([&builder, divide_front]() {
        builder.SetUpLabelArray_(divide_front);
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
inline void TailDictBuilder::LabelArrange_(size_t state) {
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
            NewFace_();
            current_dict_().set(orig_fsa_.get_trans_symbol(labelTrans));
            do {
                labelTrans = orig_fsa_.get_target_state(labelTrans);
                current_dict_().set(orig_fsa_.get_trans_symbol(labelTrans));
            } while (orig_fsa_.is_straight_state(labelTrans));
            SaveStrDict_(index);
        }
        
        LabelArrange_(orig_fsa_.get_target_state(labelTrans));
    }
    
}

void TailDictBuilder::NewFace_() {
    StrDictData dict;
    dict.id = str_dicts_.size();
    str_dicts_.push_back(dict);
    cur_str_dict_index_ = str_dicts_.size() - 1;
}

void TailDictBuilder::SaveStrDict_(size_t index) {
    auto &dict = current_dict_();
    has_label_bits_[index] = true;
    dict.node_id = index;
    dict.counter = 1;
}

void TailDictBuilder::MakeDict_() {
    fsa_target_indexes_.resize(orig_fsa_.get_num_elements());
    has_label_bits_.resize(orig_fsa_.get_num_elements());
    LabelArrange_(orig_fsa_.get_root_state());
}

void TailDictBuilder::SetSharing_(bool merge_suffix) {
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
        } else if (merge_suffix && (match > 0 && match == cLen)) {
            // included
            cur.is_included = true;
            cur.owner = owner->id;
            owner->counter++;
        } else {
            owner = &cur;
        }
    }
    
}

void TailDictBuilder::SetUpLabelArray_(bool divide_front) {
    std::sort(str_dicts_.begin(), str_dicts_.end(), [](StrDictData &lhs, StrDictData &rhs) {
        return lhs.is_included != rhs.is_included ? lhs.is_included < rhs.is_included :
        lhs.entropy() > rhs.entropy();
    });
    UpdateIdMap_();
    auto count = 0;
    for (auto& dict : str_dicts_) {
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
        if (!dict.is_included) {
            label_array_.AddString(divide_front ? dict.follows() : dict.label);
        }
    }
}

// MARK: - Log

void TailDictBuilder::ShowMappingOfByteSize_() {
    size_t counts[4] = {0, 0, 0, 0};
    for (auto& dict : str_dicts_) {
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

#endif /* TailDictBuilder_hpp */
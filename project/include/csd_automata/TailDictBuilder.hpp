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
#include "util.hpp"
#include "PlainFSA.hpp"
#include "SerializedStringsBuilder.hpp"
#include "TailDict.hpp"

#include "Director.hpp"

#include "poplar.hpp"


namespace csd_automata {

class TailDictBuilder {
    friend class TailDict;
    
    const PlainFSA& orig_fsa_;
    
    struct Container {
        id_type id = -1;
        // this label
        std::string label = "";
        // place at node
        std::vector<size_t> node_ids;
        // Data id of included owner
        id_type owner = -1;
        // label placed index at array
        int place = -1;
        // matched counter
        size_t counter = 0;
        
        bool is_merged() const {
            return owner != -1;
        }
        
        double entropy() const {
            return double(counter) / label.size();
        }
        
    };
    
    std::vector<Container> containers_;
    using StringMap = poplar::compact_hash_map<id_type>;
    StringMap string_map_;
    std::unordered_map<size_t, size_t> state_map_;
    
    std::unordered_map<size_t, size_t> trans_to_label_id_;
    SerializedStringsBuilder label_array_;
    
    
public:
    TailDictBuilder(const TailDictBuilder&) = delete;
    TailDictBuilder& operator=(const TailDictBuilder&) = delete;
    
private:
    explicit TailDictBuilder(const PlainFSA& fsa, bool binary_mode) : orig_fsa_(fsa), label_array_(binary_mode) {}
    
    void Build(bool merge_suffix);
    
    template <class Product>
    void Release(Product& dict);
    
    std::string string_reverse_(std::string text) const {
        reverse(text.begin(), text.end());
        return text;
    }
    
    // Recursive function
    void LabelArrange_(size_t state);
    
    void NewFace_();
    void SaveStrDict_(size_t index);
    
    void MakeDict_();
    void SetSharing_(bool merge_suffix);
    void SetUpLabelArray_();
    
    void ShowMappingOfByteSize_();
    
};


// MARK: - Static build function

void TailDictBuilder::Build(bool merge_suffix) {
    std::cout << "------ TailDict build bench mark ------" << std::endl;
    
    std::cout << "MakeDict: " << util::MeasureProcessing([&]() {
        MakeDict_();
    }) << "ms" << std::endl;
    
    std::cout << "SetSharings: " <<  util::MeasureProcessing([&, merge_suffix]() {
        SetSharing_(merge_suffix);
    }) << "ms" << std::endl;
    
    std::cout << "SetUpLabelArray: " << util::MeasureProcessing([&]() {
        SetUpLabelArray_();
    }) << "ms" << std::endl;
    
//    builder.showMappingOfByteSize();
}
    
    
template <class Product>
void TailDictBuilder::Release(Product& dict) {
    dict.trans_to_label_id_ = std::move(trans_to_label_id_);
    dict.label_array_ = std::move(label_array_);
}


// MARK: - Member functions

// Recusive function
void TailDictBuilder::LabelArrange_(size_t state) {
    const auto first_trans = orig_fsa_.get_first_trans(state);
    
    if (first_trans == 0 or // last trans
        state_map_.find(state) != state_map_.end()) {// already visited state
        return;
    }
    
    state_map_.insert({state, 0});
    
    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
        auto label_trans = trans;
        if (orig_fsa_.is_straight_state(label_trans)) {
            auto index = label_trans / PlainFSA::kSizeTrans;
            std::string label_on_path;
            label_on_path += orig_fsa_.get_trans_symbol(label_trans);
            do {
                label_trans = orig_fsa_.get_target_state(label_trans);
                label_on_path.push_back(orig_fsa_.get_trans_symbol(label_trans));
            } while (orig_fsa_.is_straight_state(label_trans));
            auto cr = poplar::make_char_range(label_on_path);
            auto* id_ptr = string_map_.find(cr);
            if (id_ptr == nullptr) {
                auto id = containers_.size();
                auto* id_ptr = string_map_.update(cr);
                *id_ptr = id;
                Container container;
                container.id = id;
                container.label = label_on_path;
                container.node_ids.push_back(index);
                container.counter = 1;
                containers_.push_back(container);
            } else {
                auto& container = containers_[*id_ptr];
                assert(container.id == *id_ptr);
                container.node_ids.push_back(index);
                container.counter++;
            }
        }
        LabelArrange_(orig_fsa_.get_target_state(label_trans));
    }
}

void TailDictBuilder::MakeDict_() {
    LabelArrange_(orig_fsa_.get_root_state());
    // clear memory
    string_map_ = StringMap();
    state_map_ = {};
}

void TailDictBuilder::SetSharing_(bool merge_suffix) {
    std::sort(containers_.begin(), containers_.end(), [](Container& lhs, Container& rhs) {
        return std::lexicographical_compare(lhs.label.rbegin(), lhs.label.rend(), rhs.label.rbegin(), rhs.label.rend());
    });
    
    Container dummy;
    Container* prev = &dummy;
    for (auto itr = containers_.rbegin(); itr != containers_.rend(); ++itr) {
        auto& cur = *itr;
        if (cur.label == "") {
            std::cerr << "label is empty!" << std::endl;;
            continue;
        }
        auto matches = 0;
        auto owner_label_len = prev->label.length();
        auto label_len = cur.label.length();
        while ((owner_label_len > matches and label_len > matches) and
               prev->label[owner_label_len - matches - 1] == cur.label[label_len - matches - 1]) {
            ++matches;
        }
        if (matches > 0 and matches == owner_label_len) {
            // sharing
            assert(false);
        } else if (merge_suffix and (matches > 0 and matches == label_len)) {
            // merge suffix
            cur.owner = prev->id;
            prev->counter += cur.counter;
        } else {
            prev = &cur;
        }
    }
    
}

void TailDictBuilder::SetUpLabelArray_() {
    std::sort(containers_.begin(), containers_.end(), [](auto lhs, auto rhs) {
        return (lhs.is_merged() != rhs.is_merged() ? lhs.is_merged() < rhs.is_merged() :
                lhs.counter > rhs.counter);
    });
    std::unordered_map<id_type, size_t> owner_map;
    size_t i = 0;
    for (; i < containers_.size() and !containers_[i].is_merged(); i++) {
        auto& container = containers_[i];
        auto index = label_array_.size();
        container.place = index;
        for (auto node : container.node_ids) {
            trans_to_label_id_.insert({node, index});
        }
        label_array_.SetPopuration(index);
        label_array_.AddString(container.label);
        
        owner_map.insert({container.id, i});
    }
    for (; i < containers_.size(); ++i) {
        auto& container = containers_[i];
        auto& owner = containers_[owner_map[container.owner]];
        assert(owner.place != -1);
        auto index = owner.place + owner.label.size() - container.label.size();
        container.place = index;
        for (auto node : container.node_ids) {
            trans_to_label_id_.insert({node, index});
        }
        label_array_.SetPopuration(index);
    }
}

// MARK: - Log

void TailDictBuilder::ShowMappingOfByteSize_() {
    size_t counts[4] = {0, 0, 0, 0};
    for (auto& dict : containers_) {
        if (dict.is_merged())
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

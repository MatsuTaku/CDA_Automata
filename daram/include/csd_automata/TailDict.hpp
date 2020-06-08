//
//  TailDict.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/07.
//

#ifndef TailDict_hpp
#define TailDict_hpp

#include "basic.hpp"
#include "util.hpp"
#include "PlainFSA.hpp"
#include "SerializedStrings.hpp"

#include "poplar.hpp"

namespace csd_automata {
    
    
class TailDict;


class TailDictBuilder {
private:
    const PlainFSA& orig_fsa_;
    
    struct LabelUnit {
        id_type id = -1;
        std::string label = "";
        std::vector<size_t> nodes;
        size_t counter = 0;
        
        id_type owner_id = -1;
        size_t pos = -1;
        
        LabelUnit() = default;
        LabelUnit(id_type id, std::string label, std::vector<size_t> nodes, size_t counter) : id(id), label(label), nodes(nodes), counter(counter) {}
        
        bool is_merged() const {return owner_id != -1;}
        double priority() const {return double(counter) / label.size();}
    };
    std::vector<LabelUnit> label_container_;
    std::unordered_map<size_t, size_t> trans_to_label_id_;
    SerializedStringsBuilder serialized_strings_;
    
    friend class TailDict;
    
    using StringMap = poplar::compact_fkhash_map<id_type>;
    using StateMap = std::unordered_map<size_t, size_t>;
    
public:
    TailDictBuilder(const TailDictBuilder&) = delete;
    TailDictBuilder& operator=(const TailDictBuilder&) = delete;
    
private:
    explicit TailDictBuilder(const PlainFSA& fsa, bool binary_mode) : orig_fsa_(fsa), serialized_strings_(binary_mode) {
        std::cout << "------ TailDict build bench mark ------" << std::endl;
        
        auto build_time = util::MeasureProcessing([&]() {
            Build();
        });
        
        std::cout << "Build string pool as 'Tail' in... " << build_time << "ms" << std::endl;
    }
    
    void Build() {
        LabelArrange_(orig_fsa_.get_root_state(), std::make_shared<StringMap>(), std::make_shared<StateMap>());
        
        std::sort(label_container_.begin(), label_container_.end(), [](auto& lhs, auto& rhs) {
            return std::lexicographical_compare(lhs.label.rbegin(), lhs.label.rend(), rhs.label.rbegin(), rhs.label.rend());
        });
        
        {
            LabelUnit dummy;
            LabelUnit* prev = &dummy;
            for (auto itr = label_container_.rbegin(); itr != label_container_.rend(); ++itr) {
                auto& cur = *itr;
                auto matches = 0;
                auto owner_label_len = prev->label.length();
                auto label_len = cur.label.length();
                while (owner_label_len > matches and label_len > matches and
                       prev->label[owner_label_len - matches - 1] == cur.label[label_len - matches - 1]) {
                    ++matches;
                }
                if (matches > 0 and matches == owner_label_len) {
                    // sharing
                    assert(false);
                } else if (matches > 0 and matches == label_len) {
                    // merge suffix
                    cur.owner_id = prev->id;
                    prev->counter += cur.counter;
                } else {
                    prev = &cur;
                }
            }
        }
        
        std::sort(label_container_.begin(), label_container_.end(), [](auto& lhs, auto& rhs) {
            return (lhs.is_merged() != rhs.is_merged() ? lhs.is_merged() < rhs.is_merged() :
                    lhs.counter > rhs.counter);
        });
        std::unordered_map<id_type, size_t> owner_map;
        size_t i = 0;
        for (; i < label_container_.size() and not label_container_[i].is_merged(); i++) {
            auto& container = label_container_[i];
            auto index = serialized_strings_.size();
            container.pos = index;
            for (auto node : container.nodes) {
                trans_to_label_id_.insert({node, index});
            }
            serialized_strings_.SetPopuration(index);
            serialized_strings_.AddString(container.label);
            
            owner_map.insert({container.id, i});
        }
        for (; i < label_container_.size(); ++i) {
            auto& container = label_container_[i];
            auto& owner = label_container_[owner_map.at(container.owner_id)];
            assert(owner.pos != -1);
            auto index = owner.pos + owner.label.size() - container.label.size();
            container.pos = index;
            for (auto node : container.nodes) {
                trans_to_label_id_.insert({node, index});
            }
            serialized_strings_.SetPopuration(index);
        }
    }
    
    // Recursive function
    void LabelArrange_(size_t state, std::shared_ptr<StringMap> string_map, std::shared_ptr<StateMap> state_map) {
        const auto first_trans = orig_fsa_.get_first_trans(state);
        if (state == 0 or // last trans
            state_map->find(state) != state_map->end()) {// already visited state
            return;
        }
        state_map->insert({state, 0});
        
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
                auto* id_ptr = string_map->find(cr);
                if (id_ptr == nullptr) {
                    auto id = label_container_.size();
                    *(string_map->update(cr)) = id;
                    label_container_.emplace_back(id, label_on_path, std::vector<size_t>{index}, 1);
                } else {
                    auto& container = label_container_[*id_ptr];
                    assert(container.id == *id_ptr);
                    container.nodes.push_back(index);
                    container.counter++;
                }
            }
            LabelArrange_(orig_fsa_.get_target_state(label_trans), string_map, state_map);
        }
    }
    
};


class TailDict {
private:
    std::unordered_map<size_t, size_t> trans_to_label_id_;
    SerializedStringsBuilder serialized_strings_;

public:
    TailDict(const PlainFSA& src_fsa) {
        TailDictBuilder builder(src_fsa, false);
        trans_to_label_id_ = std::move(builder.trans_to_label_id_);
        serialized_strings_ = std::move(builder.serialized_strings_);
    }

    TailDict(const TailDict&) = delete;
    TailDict& operator=(const TailDict&) = delete;

    bool check_label_codes(size_t index, size_t* code) const {
        auto target = trans_to_label_id_.find(index);
        if (target != trans_to_label_id_.end()) {
            *code = target->second;
            return true;
        } else {
            return false;
        }
    }

    friend SerializedStringsBuilder& GetSerializedStringsBuilder(TailDict& sd) {
        return sd.serialized_strings_;
    }

};

}

#endif /* TailDict_hpp */

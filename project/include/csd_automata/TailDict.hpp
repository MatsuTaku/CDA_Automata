//
//  TailDict.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/07.
//

#ifndef TailDict_hpp
#define TailDict_hpp

#include "basic.hpp"
#include "SerializedStringsBuilder.hpp"

namespace csd_automata {
    
    
struct StrDictData {
    // Elabled
    bool enabled = true;
    // This data id
    size_t id = 0;
    // place at node
    size_t node_id = 0;
    // this label
    std::string label = "";
    // matched with suffix of other label
    bool is_included = false;
    // Data id of included owner
    size_t owner = 0;
    // label placed index at array
    int place = -1;
    // matched counter
    size_t counter = 0;
    
    void push_label(char c) {
        label += c;
    }
    
    char front() const {
        return label.front();
    }
    
    std::string_view follows() const {
        return std::string_view(label).substr(1);
    }
    
    float entropy() const {
        return float(counter) / label.size();
    }
    
};
    

class TailDict {
public:
    friend class TailDictBuilder;
    
    bool has_label(size_t index) const {
        return has_label_bits_[index];
    }
    
    bool is_label_source(size_t index) const {
        return has_label(index);
    }
    
    size_t dict_trans(size_t index) const {
        assert(has_label(index));
        return fsa_target_indexes_[index];
    }
    
    const StrDictData& dict(size_t index) const {
        return str_dicts_[index];
    }
    
    size_t start_pos(size_t index) const {
        return data_(index).place;
    }
    
    bool is_back_of_label_at(size_t index) const {
        return label_array_.is_back_at(index);
    }
    
    uint8_t top_char_on_pos(size_t index) const {
        return label_array_[start_pos(index)];
    }
    
    size_t size_dicts() const {
        return str_dicts_.size();
    }
    
    friend SerializedStringsBuilder& GetSerializedStringsBuilder(TailDict& sd) {
        return sd.label_array_;
    }
    
    TailDict() = default;
    ~TailDict() = default;
    
    TailDict(const TailDict &) = delete;
    TailDict& operator=(const TailDict &) = delete;
    
    TailDict(TailDict &&) noexcept = default;
    TailDict& operator=(TailDict &&) noexcept = default;
    
private:
    std::vector<StrDictData> str_dicts_;
    std::vector<size_t> fsa_target_indexes_;
    std::vector<bool> has_label_bits_;
    SerializedStringsBuilder label_array_;
    
//    size_t pos_on_label_ = 0;
    
    const StrDictData& data_(size_t trans) const {
        return str_dicts_[dict_trans(trans)];
    }
    
};

}

#endif /* TailDict_hpp */

//
//  StringDict.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/07.
//

#ifndef StringDict_hpp
#define StringDict_hpp

#include "basic.hpp"
#include "StrDictData.hpp"
#include "SerializedStringsBuilder.hpp"

namespace csd_automata {

class StringDict {
    
    std::vector<StrDictData> str_dicts_;
    std::vector<size_t> fsa_target_indexes_;
    std::vector<bool> has_label_bits_;
    SerializedStringsBuilder label_array_;
    
    size_t pos_on_label_ = 0;
    friend class StringDictBuilder;
    
public:
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
    
    bool is_end_label() const {
        return is_back_of_label_at(pos_on_label_);
    }
    
    uint8_t top_char_on_pos(size_t index) const {
        return label_array_[start_pos(index)];
    }
    
    void pos_to_next() {
        pos_on_label_++;
    }
    
    void TraceOnLabel(size_t index) {
        pos_on_label_ = start_pos(index);
    }
    
    size_t size_dicts() const {
        return str_dicts_.size();
    }
    
    friend SerializedStringsBuilder& GetSerializedStringsBuilder(StringDict& sd) {
        return sd.label_array_;
    }
    
    StringDict() = default;
    ~StringDict() = default;
    
    StringDict(const StringDict &) = delete;
    StringDict& operator=(const StringDict &) = delete;
    
    StringDict(StringDict &&) noexcept = default;
    StringDict& operator=(StringDict &&) noexcept = default;
    
private:
    const StrDictData& data_(size_t trans) const {
        return str_dicts_[dict_trans(trans)];
    }
    
};

}

#endif /* StringDict_hpp */

//
//  TailDict.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/07.
//

#ifndef TailDict_hpp
#define TailDict_hpp

#include "basic.hpp"
#include "TailDictContainer.hpp"
#include "SerializedStringsBuilder.hpp"
#include "TailDictBuilder.hpp"

namespace csd_automata {

class TailDict {
public:
    friend class TailDictBuilder;
    using Builder = TailDictBuilder;
    
    void Build(const PlainFSA& src_fsa_, bool binary_mode, bool merge_suffix, bool divide_front) {
        Builder builder(src_fsa_, binary_mode);
        builder.Build(merge_suffix, divide_front);
        builder.Release(*this);
    }
    
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
    
    const TailDictContainer& dict(size_t index) const {
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
    std::vector<TailDictContainer> str_dicts_;
    std::vector<size_t> fsa_target_indexes_;
    std::vector<bool> has_label_bits_;
    SerializedStringsBuilder label_array_;
    
//    size_t pos_on_label_ = 0;
    
    const TailDictContainer& data_(size_t trans) const {
        return str_dicts_[dict_trans(trans)];
    }
    
};

}

#endif /* TailDict_hpp */

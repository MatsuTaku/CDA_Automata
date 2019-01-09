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
#include "TailDictBuilder.hpp"

namespace csd_automata {

class TailDict {
public:
    friend class TailDictBuilder;
    using Builder = TailDictBuilder;
    
private:
    std::unordered_map<size_t, size_t> trans_to_label_id_;
    SerializedStringsBuilder label_array_;
    
public:
    TailDict() = default;
    
    TailDict(const TailDict &) = delete;
    TailDict& operator=(const TailDict &) = delete;
    
    TailDict(TailDict&&) = default;
    TailDict& operator=(TailDict&&) = default;
    
    void Build(const PlainFSA& src_fsa_, bool binary_mode, bool merge_suffix) {
        Builder builder(src_fsa_, binary_mode);
        builder.Build(merge_suffix);
        builder.Release(*this);
    }
    
    bool check_label_codes(size_t index, size_t* code) {
        auto target = trans_to_label_id_.find(index);
        if (target != trans_to_label_id_.end()) {
            *code = target->second;
            return true;
        } else {
            return false;
        }
    }
    
    friend SerializedStringsBuilder& GetSerializedStringsBuilder(TailDict& sd) {
        return sd.label_array_;
    }
    
};

}

#endif /* TailDict_hpp */

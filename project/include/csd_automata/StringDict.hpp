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
        friend class StringDictBuilder;
    public:
        StringDict() = default;
        ~StringDict() = default;
        
        bool hasLabel(size_t index) const {
            return has_label_bits_[index];
        }
        
        bool isLabelSource(size_t index) const {
            return hasLabel(index);
        }
        
        size_t dictTrans(size_t index) const {
            assert(hasLabel(index));
            return fsa_target_indexes_[index];
        }
        
        const StrDictData& dict(size_t index) const {
            return str_dicts_[index];
        }
        
        size_t startPos(size_t index) const {
            return dataOf(index).place;
        }
        
        bool isBackOfLabelAt(size_t index) const {
            return label_array_.isBackAt(index);
        }
        
        bool isEndLabel() const {
            return isBackOfLabelAt(pos_on_label_);
        }
        
        uint8_t topCharOnPos(size_t index) const {
            return label_array_[startPos(index)];
        }
        
        void posToNext() {
            pos_on_label_++;
        }
        
        void traceOnLabel(size_t index) {
            pos_on_label_ = startPos(index);
        }
        
        size_t numDicts() const {
            return str_dicts_.size();
        }
        
        friend SerializedStringsBuilder& serializedStringsBuilder(StringDict& sd) {
            return sd.label_array_;
        }
        
        StringDict(const StringDict &) = delete;
        StringDict& operator=(const StringDict &) = delete;
        
        StringDict(StringDict &&) noexcept = default;
        StringDict& operator=(StringDict &&) noexcept = default;
        
    private:
        std::vector<StrDictData> str_dicts_;
        std::vector<size_t> fsa_target_indexes_;
        std::vector<bool> has_label_bits_;
        SerializedStringsBuilder label_array_;
        
        size_t pos_on_label_ = 0;
        
        const StrDictData& dataOf(size_t trans) const {
            return str_dicts_[dictTrans(trans)];
        }
        
    };
    
}

#endif /* StringDict_hpp */

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
#include "Rank.hpp"

namespace array_fsa {
    
    class StringDict {
        friend class StringDictBuilder;
        
    public:
        bool hasLabel(size_t index) const {
            return has_label_bits_[index];
        }
        
        bool isLabelSource(size_t index) {
            return hasLabel(index);
        }
        
        size_t startPos(size_t index) const {
            auto data = dataOf(index);
            return dataOf(index).place;
        }
        
        bool isEndLabel(size_t index) const {
            return finish_flags_[index];
        }
        
        bool isEndLabel() const {
            return finish_flags_[pos_on_label_];
        }
        
        uint8_t topCharOnPos(size_t index) const {
            return label_bytes_[startPos(index)];
        }
        
        void posToNext() {
            pos_on_label_++;
        }
        
        void traceOnLabel(size_t index) {
            pos_on_label_ = startPos(index);
        }
        
        std::vector<uint8_t> &get_label_bytes() {
            return label_bytes_;
        }
        
        std::vector<bool> &get_finish_flags() {
            return finish_flags_;
        }
        
    private:
        std::vector<StrDictData> str_dicts_;
        std::vector<size_t> fsa_target_indexes_;
        std::vector<bool> has_label_bits_;
        std::vector<uint8_t> label_bytes_;
        std::vector<bool> finish_flags_;
        
        size_t pos_on_label_ = 0;
        
        size_t dictIndex(size_t index) const {
            assert(hasLabel(index));
            return fsa_target_indexes_[index];
        }
        
        StrDictData dataOf(size_t index) const {
            const StrDictData &dict = str_dicts_[dictIndex(index)];
            return dict;
        }
        
    };
    
}

#endif /* StringDict_hpp */

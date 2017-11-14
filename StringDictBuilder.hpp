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
#include "StrDictData.hpp"
#include "Rank.hpp"
#include "ArrayTri.hpp"

namespace array_fsa {
    
    class PlainFSA;
    class StringDict;
    
    class StringDictBuilder {
    public:
        static StringDict build(const PlainFSA &fsa);
        
        StringDictBuilder(const StringDictBuilder &) = delete;
        StringDictBuilder & operator=(const StringDictBuilder &) = delete;
        
    private:
        const PlainFSA &orig_fsa_;
        
        explicit StringDictBuilder(const PlainFSA &fsa) : orig_fsa_(fsa) {}
        ~StringDictBuilder() = default;
        
        std::vector<StrDictData> str_dicts_;
        size_t cur_str_dict_index_;
        std::vector<size_t> str_dict_indexes_;
        std::vector<bool> has_label_bits_;
        std::vector<uint8_t> label_bytes_;
        std::vector<bool> finish_flags_;
        
        std::unordered_map<size_t, size_t> state_map_;
        
        ArrayTri dict_tri_;
        
        StrDictData &cur_str_dict() {
            return str_dicts_[cur_str_dict_index_];
        }
        
        void makeDict();
        void labelArrange(size_t state);
        void flatStringArray();
        
        void appendStrDict() {
            StrDictData dict;
            str_dicts_.push_back(dict);
            cur_str_dict_index_ = str_dicts_.size() - 1;
        }
        
        void saveStrDict(size_t index) {
            has_label_bits_[index] = true;
            
//            for (auto i = 0; i < cur_str_dict_index_; i++) {
//                auto &toDict = str_dicts_[i];
//                if (toDict.label == cur_str_dict().label) {
//                    str_dict_indexes_[index] = i;
//                    str_dicts_.pop_back();
//                    return;
//                }
//            }
            auto sameAs = dict_tri_.isMember(cur_str_dict().label);
            if (sameAs != 0) {
                str_dict_indexes_[index] = sameAs;
                str_dicts_.pop_back();
                return;
            }
            str_dict_indexes_[index] = cur_str_dict_index_;
            
            dict_tri_.add(cur_str_dict().label, cur_str_dict_index_);
        }
    };
    
}

#endif /* StringDictBuilder_hpp */

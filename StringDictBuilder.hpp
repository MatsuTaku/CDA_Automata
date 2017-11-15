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
        std::vector<size_t> fsa_target_ids_;
        std::vector<size_t> fsa_target_indexes_;
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
        void sortDicts();
        void flatStringArray();
        
        void appendStrDict() {
            StrDictData dict;
            dict.id = str_dicts_.size();
            str_dicts_.push_back(dict);
            cur_str_dict_index_ = str_dicts_.size() - 1;
        }
        
        void saveStrDict(size_t index) {
            has_label_bits_[index] = true;
            
            auto sameAs = dict_tri_.isMember(cur_str_dict().label);
            if (sameAs != 0) {
                str_dicts_[sameAs].counter++;
                fsa_target_ids_[index] = str_dicts_[sameAs].id;
                str_dicts_.pop_back();
                return;
            }
            fsa_target_ids_[index] = cur_str_dict().id;
            
            dict_tri_.add(cur_str_dict().label, cur_str_dict_index_);
        }
        
    };
    
}

#endif /* StringDictBuilder_hpp */

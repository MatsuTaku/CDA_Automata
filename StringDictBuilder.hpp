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
    };
    
}

#endif /* StringDictBuilder_hpp */

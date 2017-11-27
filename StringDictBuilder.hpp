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
        
        std::unordered_map<size_t, size_t> state_map_;
        
//        ArrayTri dict_tri_;
        ArrayTri dict_reverse_tri_;
        std::vector<size_t> idMap;
        
        void updateIdMap() {
            idMap = {};
            idMap.resize(str_dicts_.size());
            
            for (auto i = 0; i < str_dicts_.size(); i++) {
                auto &dict = str_dicts_[i];
                idMap[dict.id] = i;
            }
        }
        
        StrDictData & getDictFromId(size_t id) {
            return str_dicts_[idMap[id]];
        }
        
        StrDictData & cur_str_dict() {
            return str_dicts_[cur_str_dict_index_];
        }
        
        std::string reverseString(std::string text) const {
            reverse(text.begin(), text.end());
            return text;
        }
        
        void makeDict();
        void labelArrange(size_t state);
        void appendStrDict();
        void saveStrDict(size_t index);
        void tailBound();
        void setIncludedOwner(StrDictData &dict);
        void sortDicts();
        void flatStringArray();
        void decideTargetIndexes();
        
        void showMappingOfByteSize();
        
    };
    
}

#endif /* StringDictBuilder_hpp */

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
#include "StringArrayBuilder.hpp"

#include "StringDict.hpp"
#include "PlainFSA.hpp"

namespace array_fsa {
    
    class StringDictBuilder {
    public:
        static StringDict build(const PlainFSA &fsa, bool binaryMode, bool mergeSuffix);
        
        ~StringDictBuilder() = default;
        
        StringDictBuilder(const StringDictBuilder &) = delete;
        StringDictBuilder& operator=(const StringDictBuilder&) = delete;
        
    private:
        const PlainFSA &orig_fsa_;
        
        explicit StringDictBuilder(const PlainFSA &fsa, bool binaryMode) : orig_fsa_(fsa), label_array_(binaryMode) {}
        
        std::vector<StrDictData> str_dicts_;
        size_t cur_str_dict_index_;
        std::vector<size_t> fsa_target_indexes_;
        std::vector<bool> has_label_bits_;
        StringArrayBuilder label_array_;
        std::unordered_map<size_t, size_t> state_map_;
        
        std::vector<size_t> idMap;
        
        std::string reverseString(std::string text) const {
            reverse(text.begin(), text.end());
            return text;
        }
        
        void updateIdMap() {
            idMap = {};
            idMap.resize(str_dicts_.size());
            for (size_t i = 0, size = str_dicts_.size(); i < size; i++)
                idMap[str_dicts_[i].id] = i;
        }
        
        StrDictData & getDictFromId(size_t id) {
            return str_dicts_[idMap[id]];
        }
        
        StrDictData & curStrDict() {
            return str_dicts_[cur_str_dict_index_];
        }
        
        // Recusive function
        void labelArrange(size_t state);
        
        void appendStrDict();
        void saveStrDict(size_t index);
        
        void makeDict();
        void setSharings(bool mergeSuffix);
        void setUpLabelArray();
        
        void showMappingOfByteSize();
        
    };
    
    
    inline StringDict StringDictBuilder::build(const PlainFSA &fsa, bool binaryMode, bool mergeSuffix) {
        StringDictBuilder builder(fsa, binaryMode);
        std::cout << "------ StringDict build bench mark ------" << std::endl;
        Stopwatch sw;
        
        builder.makeDict();
        double lastSW, newSW = sw.get_milli_sec();
        std::cout << "makeDict: " << newSW << "µs" << std::endl;
        lastSW = newSW;
        builder.setSharings(mergeSuffix);
        newSW = sw.get_milli_sec();
        std::cout << "setSharings: " << newSW - lastSW << "µs" << std::endl;
        lastSW = newSW;
        builder.setUpLabelArray();
        newSW = sw.get_milli_sec();
        std::cout << "setUpLabelArray: " << newSW - lastSW << "µs" << std::endl;
        
        builder.showMappingOfByteSize();
        
        StringDict dict;
        dict.str_dicts_ = builder.str_dicts_;
        dict.fsa_target_indexes_ = builder.fsa_target_indexes_;
        dict.has_label_bits_ = builder.has_label_bits_;
        dict.label_array_ = std::move(builder.label_array_);
        return dict;
        }
    
}

#endif /* StringDictBuilder_hpp */

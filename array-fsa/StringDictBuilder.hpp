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
#include "StringDict.hpp"
#include "BitVector.hpp"
#include "ArrayTri.hpp"
#include "StringArrayBuilder.hpp"
#include "StringArray.hpp"
#include "PlainFSA.hpp"

namespace array_fsa {
    
    class StringDictBuilder {
    public:
        static StringDict build(const PlainFSA &fsa, bool binaryMode, bool mergeSuffix) {
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
        
        StringDictBuilder(const StringDictBuilder &) = delete;
        StringDictBuilder& operator=(const StringDictBuilder &) = delete;
        
    private:
        const PlainFSA &orig_fsa_;
        
        explicit StringDictBuilder(const PlainFSA &fsa, bool binaryMode) : orig_fsa_(fsa), label_array_(binaryMode) {}
        ~StringDictBuilder() = default;
        
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
            for (size_t i = 0, size = str_dicts_.size(); i < size; i++) {
                idMap[str_dicts_[i].id] = i;
            }
        }
        
        StrDictData & getDictFromId(size_t id) {
            return str_dicts_[idMap[id]];
        }
        
        StrDictData & curStrDict() {
            return str_dicts_[cur_str_dict_index_];
        }
        
        void labelArrange(size_t state) {
            const auto first_trans = orig_fsa_.get_first_trans(state);
            
            if (first_trans == 0 || // last trans
                state_map_.find(state) != state_map_.end()) {// already visited state
                return;
            }
            
            state_map_.insert(std::make_pair(state, 0));
            
            for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
                auto labelTrans = trans;
                if (orig_fsa_.is_straight_state(labelTrans)) {
                    auto index = labelTrans / PlainFSA::kTransSize;
                    appendStrDict();
                    curStrDict().set(orig_fsa_.get_trans_symbol(labelTrans));
                    do {
                        labelTrans = orig_fsa_.get_target_state(labelTrans);
                        curStrDict().set(orig_fsa_.get_trans_symbol(labelTrans));
                    } while (orig_fsa_.is_straight_state(labelTrans));
                    saveStrDict(index);
                }
                
                labelArrange(orig_fsa_.get_target_state(labelTrans));
            }
        }
        void appendStrDict();
        void saveStrDict(size_t index);
        
        void makeDict();
        void setSharings(bool mergeSuffix);
        void setUpLabelArray();
        
        void showMappingOfByteSize();
        
    };
    
    inline void StringDictBuilder::appendStrDict() {
        StrDictData dict;
        dict.id = str_dicts_.size();
        str_dicts_.push_back(dict);
        cur_str_dict_index_ = str_dicts_.size() - 1;
    }
    
    inline void StringDictBuilder::saveStrDict(size_t index) {
        auto &dict = curStrDict();
        has_label_bits_[index] = true;
        dict.node_id = index;
        dict.counter = 1;
    }
    
}

#endif /* StringDictBuilder_hpp */

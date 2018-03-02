//
//  ArrayFSATailBuilder.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#ifndef ArrayFSA_TailBuilder_hpp
#define ArrayFSA_TailBuilder_hpp

#include "ArrayFSABuilder.hpp"
#include "BitVector.hpp"
#include "StringDict.hpp"
#include "StringDictBuilder.hpp"
#include "StringTransFSA.hpp"

namespace array_fsa {
    
    class ArrayFSATailBuilder : public ArrayFSABuilder {
    public:
        template <class T>
        static T build(const PlainFSA &origFsa) {
            const auto isBinary = T::SAType::kBinaryMode;
            ArrayFSATailBuilder builder(origFsa, isBinary);
            
            // Release
            T newFsa;
            
            const auto numElems = builder.num_elems_();
            newFsa.setNumElement(numElems);
            newFsa.strings_ = StringArray<isBinary>(&labelArray(builder.str_dict_));
            newFsa.setNumStrings(newFsa.strings_.size());
            
            auto numTrans = 0;
            for (auto i = 0; i < numElems; i++) {
                newFsa.setNextAndIsFinal(i, builder.get_next_(i), builder.is_final_(i));
                newFsa.setCheck(i, builder.get_check_(i));
                auto isStrTrans = builder.has_label(i);
                newFsa.setIsStringTrans(i, isStrTrans);
                if (isStrTrans) {
                    newFsa.setStringIndex(i, builder.get_label_number(i));
                } else {
                    newFsa.setCheck(i, builder.get_check_(i));
                }
                
                if (builder.is_frozen_(i))
                    numTrans++;
            }
            newFsa.buildBitArray();
            newFsa.setNumTrans(numTrans);
            
            //    showInBox(builder, newFsa);
            
            return newFsa;
        }
        
        template <class T>
        void showInBox(T &fsa);
        
    protected:
        StringDict str_dict_;
        
        explicit ArrayFSATailBuilder(const PlainFSA &orig_fsa, bool binaryMode) : ArrayFSABuilder(orig_fsa) {
            str_dict_ = StringDictBuilder::build(orig_fsa_, binaryMode);
            build_(binaryMode);
        }
        
        virtual ~ArrayFSATailBuilder() = default;
        
        size_t get_label_number(size_t index) const {
            size_t labelIndex = 0;
            std::memcpy(&labelIndex, &bytes_[offset_(index) + 1 + kAddrSize], 4);
            return labelIndex;
        }
        
        bool has_label(size_t index) const {
            return (bytes_[offset_(index)] & 8) != 0;
        }
        
        bool is_label_finish(size_t index) const {
            return str_dict_.isEndLabel(index);
        }
        
        void set_label_index_(size_t index, size_t labelIndex) {
            std::memcpy(&bytes_[offset_(index) + 1 + kAddrSize], &labelIndex, 4);
        }
        
        void set_has_label(size_t index) {
            bytes_[offset_(index)] |= 8;
        }
        
        void build_(bool binaryMode);
        
        void arrange_(size_t state, size_t index) override {
            const auto first_trans = orig_fsa_.get_first_trans(state);
            
            if (first_trans == 0) {
                set_next_(index, index); // to the terminal state
                return;
            }
            
            { // Set next of offset to state's second if needed.
                auto it = state_map_.find(state);
                if (it != state_map_.end()) {
                    // already visited state
                    set_next_(index, it->second);
                    return;
                }
            }
            
            const auto next = find_next_(first_trans);
            if (offset_(next) >= bytes_.size()) {
                expand_();
            }
            
            set_next_(index, next);
            state_map_.insert(std::make_pair(state, next));
            set_used_next_(next, true);
            
            for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
                const auto symbol = orig_fsa_.get_trans_symbol(trans);
                const auto child_index = next ^ symbol;
                
                freeze_state_(child_index);
                
                auto transIndex = trans / PlainFSA::kTransSize;
                if (str_dict_.isLabelSource(transIndex)) {
                    set_has_label(child_index);
                    set_label_index_(child_index, str_dict_.startPos(transIndex));
                    
                    auto labelTrans = trans;
                    str_dict_.traceOnLabel(labelTrans / PlainFSA::kTransSize);
                    while (!str_dict_.isEndLabel() && orig_fsa_.is_straight_state(labelTrans)) {
                        labelTrans = orig_fsa_.get_target_state(labelTrans);
                        str_dict_.posToNext();
                    }
                    if (orig_fsa_.is_final_trans(labelTrans)) {
                        set_final_(child_index, true);
                    }
                } else {
                    set_check_(child_index, symbol);
                    
                    if (orig_fsa_.is_final_trans(trans)) {
                        set_final_(child_index, true);
                    }
                }
            }
            
            for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
                const auto symbol = orig_fsa_.get_trans_symbol(trans);
                
                auto labelTrans = trans;
                auto transIndex = labelTrans / PlainFSA::kTransSize;
                if (str_dict_.isLabelSource(transIndex)) {
                    str_dict_.traceOnLabel(transIndex);
                    while (!str_dict_.isEndLabel() && orig_fsa_.is_straight_state(labelTrans)) {
                        labelTrans = orig_fsa_.get_target_state(labelTrans);
                        str_dict_.posToNext();
                    }
                }
                
                arrange_(orig_fsa_.get_target_state(labelTrans), next ^ symbol);
            }
        }
        
    };
    
}

#endif /* ArrayFSA_TailBuilder_hpp */

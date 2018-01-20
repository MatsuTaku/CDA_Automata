//
//  ArrayFSATailBuilder.cpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#include "ArrayFSATailBuilder.hpp"

#include "PlainFSA.hpp"
#include "NArrayFSATextEdge.hpp"
#include "StringDictBuilder.hpp"
#include "StringDict.hpp"

#include "StringTransFSA.hpp"

using namespace array_fsa;

//NArrayFSATextEdge ArrayFSATailBuilder::buildNArrayFSATextEdge(const PlainFSA &origFsa) {
//    ArrayFSATailBuilder builder(origFsa);
//    builder.build_();
//
//    // Release
//    NArrayFSATextEdge newFsa;
//
//    const auto numElems = builder.num_elems_();
//    auto nextSize = Calc::sizeFitInBytes((numElems - 1) << 1);
//    newFsa.setValuesSizes(nextSize, 1);
//    newFsa.byte_array_.resize(numElems);
//    newFsa.labels_ = builder.str_dict_.get_label_bytes();
//    auto labelIndexSize = Calc::sizeFitInBytes(newFsa.labels_.size() - 1);
//    newFsa.label_index_flows_.setUnitSize(labelIndexSize - 1);
//    newFsa.label_index_flows_.setMaxValue((newFsa.labels_.size() - 1) >> 8);
//    newFsa.label_index_flows_.useLink(true);
//
//    for (size_t i = 0; i < numElems; ++i) {
//        if (!builder.has_label(i)) {
//            newFsa.setCheck(i, builder.get_check_(i));
//        } else {
//            newFsa.setHasLabel(i, true);
//            newFsa.setLabelIndex(i, builder.get_label_number(i));
//        }
//        newFsa.setNextAndFinal(i, builder.get_next_(i), builder.is_final_(i));
//
//        if (builder.is_frozen_(i)) {
//            newFsa.num_trans_++;
//        }
//    }
//    newFsa.label_index_flows_.build();
//
//    //    showInBox(builder, newFsa);
//
//    return newFsa;
//}

template <bool B>
StringTransFSA<B> ArrayFSATailBuilder::build(const PlainFSA& origFsa) {
    ArrayFSATailBuilder builder(origFsa);
    builder.build_();
    
    // Release
    StringTransFSA<B> newFsa;
    
    const auto numElems = builder.num_elems_();
    newFsa.setNumElement(numElems);
    {
        auto& strings = builder.str_dict_.get_label_bytes();
        newFsa.setNumStrings(strings.size());
        newFsa.setStringArray(strings);
    }
    
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

template StringTransFSA<false> ArrayFSATailBuilder::build(const array_fsa::PlainFSA &);
template StringTransFSA<true> ArrayFSATailBuilder::build(const array_fsa::PlainFSA &);

template <class T>
void ArrayFSATailBuilder::showInBox(array_fsa::ArrayFSATailBuilder &builder, T &fsa) {
    auto tab = "\t";
    for (auto i = 0; i < 0x100; i++) {
        std::cout << i << tab << builder.is_final_(i) << tab << builder.get_next_(i) << tab << builder.get_check_(i) << tab << builder.has_label(i) << std::endl;
        std::cout << i << tab << fsa.isFinal(i) << tab << fsa.next(i) << tab << fsa.check(i) << tab << fsa.isStringTrans(i) << std::endl;
        if (builder.has_label(i)) {
            Rank::show_as_bytes(builder.get_label_number(i), 4);
            Rank::show_as_bytes(fsa.stringIndex(i), 4);
        }
        std::cout << std::endl;
    }
}

// MARK: - private

void ArrayFSATailBuilder::build_() {
    
    bytes_.reserve(static_cast<size_t>(orig_fsa_.get_num_trans() * 1.1 * kElemSize));
    
    expand_();
    freeze_state_(0);
    set_true_final_and_used_next_(0);
    
    // TODO: string dict
    str_dict_ = StringDictBuilder::build(orig_fsa_);
    
    arrange_(orig_fsa_.get_root_state(), 0);
    
    // reset
    state_map_ = std::unordered_map<size_t, size_t>();
}

void ArrayFSATailBuilder::arrange_(size_t state, size_t index) {
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

void ArrayFSATailBuilder::expand_() {
    ArrayFSABuilder::expand_();
}


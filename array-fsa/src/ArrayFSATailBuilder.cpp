//
//  ArrayFSATailBuilder.cpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#include "array_fsa/ArrayFSATailBuilder.hpp"
#include "array_fsa/StringTransFSA.hpp"

#include "sim_ds/Log.hpp"

using namespace array_fsa;

// MARK: - private

void ArrayFSATailBuilder::build_(bool binarymode) {
    bytes_.reserve(static_cast<size_t>(orig_fsa_.get_num_trans() * 1.1 * kElemSize));
    
    expand_();
    freezeState_(0);
    setFinalAndUsedNext_(0);
    
    arrange_(orig_fsa_.get_root_state(), 0);
    
    // reset
    state_map_ = std::unordered_map<size_t, size_t>();
}

void ArrayFSATailBuilder::arrange_(size_t state, size_t index) {
    const auto first_trans = orig_fsa_.get_first_trans(state);
    
    if (first_trans == 0) {
        setNext_(index, index); // to the terminal state
        return;
    }
    
    { // Set next of offset to state's second if needed.
        auto it = state_map_.find(state);
        if (it != state_map_.end()) {
            // already visited state
            setNext_(index, it->second);
            return;
        }
    }
    
    const auto next = findNext_(first_trans);
    if (offset_(next) >= bytes_.size()) {
        expand_();
    }
    
    setNext_(index, next);
    state_map_.insert(std::make_pair(state, next));
    setUsedNext_(next, true);
    
    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
        const auto symbol = orig_fsa_.get_trans_symbol(trans);
        const auto child_index = next ^ symbol;
        
        freezeState_(child_index);
        
        auto transIndex = trans / PlainFSA::kTransSize;
        if (str_dict_.isLabelSource(transIndex)) {
            setHasLabel_(child_index);
            setLabelIndex_(child_index, str_dict_.startPos(transIndex));
            
            auto labelTrans = trans;
            str_dict_.traceOnLabel(labelTrans / PlainFSA::kTransSize);
            while (!str_dict_.isEndLabel() && orig_fsa_.is_straight_state(labelTrans)) {
                labelTrans = orig_fsa_.get_target_state(labelTrans);
                str_dict_.posToNext();
            }
            if (orig_fsa_.is_final_trans(labelTrans)) {
                setFinal_(child_index, true);
            }
        } else {
            setCheck_(child_index, symbol);
            
            if (orig_fsa_.is_final_trans(trans)) {
                setFinal_(child_index, true);
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


//
//  ArrayFSABuilder.cpp
//
//  Created by 松本拓真 on 2017/10/18.
//

#include "array_fsa/ArrayFSABuilder.hpp"

#include "array_fsa/PlainFSA.hpp"
#include "array_fsa/FSA.hpp"
#include "sim_ds/Calc.hpp"

using namespace array_fsa;


// MARK: - private

void ArrayFSABuilder::build_() {
    bytes_.reserve(static_cast<size_t>(orig_fsa_.get_num_trans() * 1.1 * kElemSize));
    
    expand_();
    freezeState_(0);
    setFinalAndUsedNext_(0);
    
    arrange_(orig_fsa_.get_root_state(), 0);
    
    // reset
    state_map_ = std::unordered_map<size_t, size_t>();
}

void ArrayFSABuilder::expand_() {
    const auto begin = index_(bytes_.size());
    const auto end = begin + kBlockSize;
    
    bytes_.resize(offset_(end), 0);
    
    for (auto i = begin; i < end; i++) {
        setSucc_(i, i + 1);
        setPred_(i, i - 1);
    }
    
    if (unfrozen_head_ == 0) {
        // initial or full
        setPred_(begin, end - 1);
        setSucc_(end - 1, begin);
        unfrozen_head_ = begin;
    } else {
        const auto unfrozen_tail = getPred_(unfrozen_head_);
        setPred_(begin, unfrozen_tail);
        setSucc_(end - 1, unfrozen_head_);
        setSucc_(unfrozen_tail, begin);
        setPred_(unfrozen_head_, end - 1);
    }
    
    if (kFreeBytes <= offset_(begin)) {
        closeBlock_(begin - index_(kFreeBytes));
    }
}

void ArrayFSABuilder::freezeState_(size_t index) {
    assert(!isFrozen_(index));
    
    setFrozen_(index, true);
    
    const auto succ = getSucc_(index);
    const auto pred = getPred_(index);
    
    // unlink
    setSucc_(pred, succ);
    setPred_(succ, pred);
    
    // set succ and pred to 0
    std::memset(&bytes_[offset_(index) + 1], 0, kAddrSize * 2);
    
    if (index == unfrozen_head_) {
        unfrozen_head_ = succ == index ? 0 : succ;
    }
}

void ArrayFSABuilder::closeBlock_(size_t begin) {
    const auto end = begin + kBlockSize;
    
    if (unfrozen_head_ == 0 || unfrozen_head_ >= end) {
        return;
    }
    for (auto i = begin; i < end; i++) {
        if (isFrozen_(i)) {
            continue;
        }
        freezeState_(i);
        setFrozen_(i, false);
    }
}

// so-called XCHECK
size_t ArrayFSABuilder::findNext_(size_t first_trans) const {
    const auto symbol = orig_fsa_.get_trans_symbol(first_trans);
    
    if (unfrozen_head_ != 0) {
        auto unfrozen_index = unfrozen_head_;
        do {
            const auto next = unfrozen_index ^symbol; // TODO: omit index_()
            if (checkNext_(next, first_trans)) {
                return next;
            }
            unfrozen_index = getSucc_(unfrozen_index);
        } while (unfrozen_index != unfrozen_head_);
    }
    
    return index_(bytes_.size()) ^ symbol;
}

bool ArrayFSABuilder::checkNext_(size_t next, size_t trans) const {
    if (isUsedNext_(next)) {
        return false;
    }
    
    do {
        const auto index = next ^orig_fsa_.get_trans_symbol(trans);
        if (isFrozen_(index)) {
            return false;
        }
        trans = orig_fsa_.get_next_trans(trans);
    } while (trans != 0);
    
    return true;
}

// Recusive function
inline void ArrayFSABuilder::arrange_(size_t state, size_t index) {
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
        setCheck_(child_index, symbol);
        
        if (orig_fsa_.is_final_trans(trans)) {
            setFinal_(child_index, true);
        }
    }
    
    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
        const auto symbol = orig_fsa_.get_trans_symbol(trans);
        arrange_(orig_fsa_.get_target_state(trans), next ^ symbol);
    }
}

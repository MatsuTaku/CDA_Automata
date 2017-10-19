//
//  ArrayFSABuilder.cpp
//
//  Created by 松本拓真 on 2017/10/18.
//

#include <stdio.h>
#include "ArrayFSABuilder.hpp"

#include "PlainFSA.hpp"

using namespace array_fsa;

// MARK: - public static

ArrayFSA ArrayFSABuilder::build(const PlainFSA& orig_fsa) {
    ArrayFSABuilder builder(orig_fsa);
    builder.build_();
    
    // Release
    ArrayFSA new_fsa;
    const auto num_elems = builder.bytes_.size() / kElemSize;
    
    new_fsa.calc_next_size(num_elems);
    new_fsa.element_size_ = new_fsa.next_size_ + 1;
    
    new_fsa.bytes_.resize(num_elems * new_fsa.element_size_);
    
    for (size_t i = 0; i < num_elems; ++i) {
        const auto offset = i * kElemSize;
        const auto new_offset = i * new_fsa.element_size_;
        
        // Set final flag to top of next bit.
        size_t next = builder.get_next_(offset) << 1 | (builder.is_final_(offset) ? 1 : 0);
        std::memcpy(&new_fsa.bytes_[new_offset], &next, new_fsa.next_size_);
        new_fsa.bytes_[new_offset + new_fsa.next_size_] = builder.get_check_(offset);
        
        if (builder.is_frozen_(offset)) {
            ++new_fsa.num_trans_;
        }
    }
    
    builder.showNextSizeMapping();
    
    return new_fsa;
}


// MARK: - public

void ArrayFSABuilder::showNextSizeMapping() {
    std::vector<size_t> next_map;
    next_map.resize(4, 0);
    for (size_t i = 0; i < bytes_.size(); i += kElemSize) {
        if (get_check_(i) == 0) {
            continue;
        }
        auto next = get_next_(i);
        int size = 0;
        while (next >> (8 * ++size - 1));
        next_map[size - 1]++;
    }
    
    std::cout << "Next size mapping" << std::endl;
    std::cout << "num_elems " << num_elems_() << std::endl;
    std::cout << "\t1\t2\t3\t4\tbyte size" << std::endl;
    auto tab = "\t";
    for (auto num: next_map) {
        auto per_num = int(double(num) / num_elems_() * 100);
        std::cout << tab << per_num;
    }
    std::cout << tab << "%";
    std::cout << std::endl;
}


// MARK: - private

void ArrayFSABuilder::build_() {
    bytes_.reserve(static_cast<size_t>(orig_fsa_.get_num_trans() * 1.1 * kElemSize));
    
    expand_();
    freeze_state_(0);
    set_true_final_and_used_next_(0);
    
    arrange_(orig_fsa_.get_root_state(), 0);
    
    // reset
    state_map_ = std::unordered_map<size_t, size_t>();
}

void ArrayFSABuilder::expand_() {
    const auto begin = bytes_.size();
    const auto end = begin + kBlockSize * kElemSize;
    
    bytes_.resize(end, 0);
    
    for (auto i = begin; i < end; i += kElemSize) {
        set_succ_(i, i + kElemSize);
        set_pred_(i, i - kElemSize);
    }
    
    if (unfrozen_head_ == 0) {
        // initial or full
        set_pred_(begin, end - kElemSize);
        set_succ_(end - kElemSize, begin);
        unfrozen_head_ = begin;
    } else {
        const auto unfrozen_tail = get_pred_(unfrozen_head_);
        set_pred_(begin, unfrozen_tail);
        set_succ_(end - kElemSize, unfrozen_head_);
        set_succ_(unfrozen_tail, begin);
        set_pred_(unfrozen_head_, end - kElemSize);
    }
    
    if (kFreeBytes <= begin) {
        close_block_(begin - kFreeBytes);
    }
}

void ArrayFSABuilder::freeze_state_(size_t offset) {
    assert(!is_frozen_(offset));
    
    set_frozen_(offset, true);
    
    const auto succ = get_succ_(offset);
    const auto pred = get_pred_(offset);
    
    // unlink
    set_succ_(pred, succ);
    set_pred_(succ, pred);
    
    // set succ and pred to 0
    std::memset(&bytes_[offset + 1], 0, kAddrSize * 2);
    
    if (offset == unfrozen_head_) {
        unfrozen_head_ = succ == offset ? 0 : succ;
    }
}

void ArrayFSABuilder::close_block_(size_t begin) {
    const auto end = begin + kBlockSize * kElemSize;
    
    if (unfrozen_head_ == 0 || unfrozen_head_ >= end) {
        return;
    }
    for (auto i = begin; i < end; i += kElemSize) {
        if (is_frozen_(i)) {
            continue;
        }
        freeze_state_(i);
        set_frozen_(i, false);
    }
}

void ArrayFSABuilder::arrange_(size_t state, size_t offset) {
    const auto first_trans = orig_fsa_.get_first_trans(state);
    
    if (first_trans == 0) {
        set_next_(offset, 0); // to the terminal state
        return;
    }
    
    { // Set next of offset to state's second if needed.
        auto it = state_map_.find(state);
        if (it != state_map_.end()) {
            // already visited state
            set_next_(offset, it->second);
            return;
        }
    }
    
    const auto next = find_next_(first_trans);
    if (offset_(next) >= bytes_.size()) {
        expand_();
    }
    
    set_next_(offset, next);
    state_map_.insert(std::make_pair(state, next));
    set_used_next_(offset_(next), true);
    
    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
        const auto symbol = orig_fsa_.get_trans_symbol(trans);
        const auto child_offset = offset_(next ^ symbol);
        
        freeze_state_(child_offset);
        set_check_(child_offset, symbol);
        
        if (orig_fsa_.is_final_trans(trans)) {
            set_final_(child_offset, true);
        }
    }
    
    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
        const auto symbol = orig_fsa_.get_trans_symbol(trans);
        arrange_(orig_fsa_.get_target_state(trans), offset_(next ^ symbol));
    }
}

// so-called XCHECK
size_t ArrayFSABuilder::find_next_(size_t first_trans) const {
    const auto symbol = orig_fsa_.get_trans_symbol(first_trans);
    
    if (unfrozen_head_ != 0) {
        auto unfrozen_offset = unfrozen_head_;
        do {
            const auto next = index_(unfrozen_offset) ^symbol; // TODO: omit index_()
            if (check_next_(next, first_trans)) {
                return next;
            }
            unfrozen_offset = get_succ_(unfrozen_offset);
        } while (unfrozen_offset != unfrozen_head_);
    }
    
    return index_(bytes_.size()) ^ symbol;
}

bool ArrayFSABuilder::check_next_(size_t next, size_t trans) const {
    if (is_used_next_(offset_(next))) {
        return false;
    }
    
    do {
        const auto index = next ^orig_fsa_.get_trans_symbol(trans);
        if (is_frozen_(offset_(index))) {
            return false;
        }
        trans = orig_fsa_.get_next_trans(trans);
    } while (trans != 0);
    
    return true;
}

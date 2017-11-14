//
//  ArrayDACFSABuilder.cpp
//
//  Created by 松本拓真 on 2017/10/18.
//

#include <stdio.h>
#include "ArrayDACFSABuilder.hpp"

#include "PlainFSA.hpp"

using namespace array_fsa;

// MARK: - public static

ArrayDACFSA ArrayDACFSABuilder::build(const PlainFSA& orig_fsa) {
    ArrayDACFSABuilder builder(orig_fsa);
    builder.build_();
    
    // Release
    ArrayDACFSA new_fsa;
    const auto num_elems = builder.bytes_.size() / kElemSize;
    
    new_fsa.calc_next_size(num_elems);
    new_fsa.element_size_ = 1 + 1; // TODO: DAC
    
    new_fsa.bytes_.resize(num_elems * new_fsa.element_size_);
    
    for (size_t i = 0; i < num_elems; ++i) {
        // TODO: DAC
        new_fsa.set_next(i, builder.get_next_(i));
        if (builder.is_final_(i)) {
            new_fsa.set_is_final(i);
        }
        new_fsa.set_check(i, builder.get_check_(i));
        
        if (builder.is_frozen_(i)) {
            ++new_fsa.num_trans_;
        }
    }
    
    new_fsa.buildBits();
    
    builder.showMapping(false);
    ArrayDACFSABuilder::showInBox(builder, new_fsa);
    
    return new_fsa;
}


// MARK: - public

void ArrayDACFSABuilder::showMapping(bool show_density) {
    auto tab = "\t";
    
    std::vector<size_t> next_map;
    next_map.resize(4, 0);
    std::vector<size_t> def_next_map;
    def_next_map.resize(4, 0);
    std::vector<size_t> dens_map;
    const auto dens_block_size = 0x100;
    dens_map.resize(num_elems_() / dens_block_size, 0);
    
    for (size_t i = 0, num_node = 0; i < index_(bytes_.size()); i++) {
        if (!is_frozen_(i)) {
            continue;
        }
        
        num_node++;
        if ((i + 1) % dens_block_size == 0) {
            dens_map[(i + 1) / dens_block_size - 1] = double(num_node) / dens_block_size * 1000;
            num_node = 0;
        }
        
        auto index = get_target_index(i);
        auto next = get_next_(i);
        int size = 0;
        while (next >> (8 * ++size));
        next_map[size - 1]++;
        size = 0;
        while (index >> (8 * ++size));
        def_next_map[size - 1]++;
    }
    
    std::cout << "Next size mapping" << std::endl;
    std::cout << "num_elems " << num_elems_() << std::endl;
    std::cout << "\t1\t2\t3\t4\tbyte size" << std::endl;
    for (auto num: def_next_map) {
        auto per_num = int(double(num) / num_elems_() * 100);
        std::cout << tab << per_num;
    }
    std::cout << tab << "%";
    std::cout << std::endl;
    for (auto num: next_map) {
        auto per_num = int(double(num) / num_elems_() * 100);
        std::cout << tab << per_num;
    }
    std::cout << tab << "%";
    std::cout << std::endl;
    
    if (show_density) {
        std::cout << "Mapping density" << std::endl;
        for (auto i = 0; i < dens_map.size(); i++) {
            if (i != 0 && i % 8 == 0) { std::cout << std::endl; }
            std::cout << tab << double(dens_map[i]) / 10 << "%";
        }
        std::cout << std::endl;
    }
}

void ArrayDACFSABuilder::showInBox(ArrayDACFSABuilder &builder, ArrayDACFSA &fsa) {
    auto tab = "\t";
    for (auto i = 0; i < 0x100; i++) {
        auto bN = builder.get_next_(i);
        auto nN = fsa.get_next_(i);
        if (bN != nN) {
            std::cout << i << tab << builder.is_final_(i) << tab << bN << std::endl;
            Rank::show_as_bytes(bN, 4);
            std::cout << i << tab << fsa.is_final_trans(i) << tab << nN << std::endl;;
            Rank::show_as_bytes(nN, 4);
            std::cout << std::endl;
        }
    }
}


// MARK: - private

void ArrayDACFSABuilder::build_() {
    bytes_.reserve(static_cast<size_t>(orig_fsa_.get_num_trans() * 1.1 * kElemSize));
    
    expand_();
    freeze_state_(0);
    set_true_final_and_used_next_(0);
    
    arrange_(orig_fsa_.get_root_state(), 0);
    
    // reset
    state_map_ = std::unordered_map<size_t, size_t>();
}

void ArrayDACFSABuilder::expand_() {
    const auto begin = index_(bytes_.size());
    const auto end = begin + kBlockSize;
    
    bytes_.resize(offset_(end), 0);
    
    for (auto i = begin; i < end; i++) {
        set_succ_(i, i + 1);
        set_pred_(i, i - 1);
    }
    
    if (unfrozen_head_ == 0) {
        // initial or full
        set_pred_(begin, end - 1);
        set_succ_(end - 1, begin);
        unfrozen_head_ = begin;
    } else {
        const auto unfrozen_tail = get_pred_(unfrozen_head_);
        set_pred_(begin, unfrozen_tail);
        set_succ_(end - 1, unfrozen_head_);
        set_succ_(unfrozen_tail, begin);
        set_pred_(unfrozen_head_, end - 1);
    }
    
    if (kFreeBytes <= offset_(begin)) {
        close_block_(begin - index_(kFreeBytes));
    }
}

void ArrayDACFSABuilder::freeze_state_(size_t index) {
    assert(!is_frozen_(index));
    
    set_frozen_(index);
    
    const auto succ = get_succ_(index);
    const auto pred = get_pred_(index);
    
    // unlink
    set_succ_(pred, succ);
    set_pred_(succ, pred);
    
    // set succ and pred to 0
    std::memset(&bytes_[offset_(index) + 1], 0, kAddrSize * 2);
    
    if (index == unfrozen_head_) {
        unfrozen_head_ = succ == index ? 0 : succ;
    }
}

void ArrayDACFSABuilder::close_block_(size_t begin) {
    const auto end = begin + kBlockSize;
    
    if (unfrozen_head_ == 0 || unfrozen_head_ >= end) {
        return;
    }
    for (auto i = begin; i < end; i++) {
        if (is_frozen_(i)) {
            continue;
        }
        freeze_state_(i);
        set_frozen_(i);
    }
}

void ArrayDACFSABuilder::arrange_(size_t state, size_t index) {
    const auto first_trans = orig_fsa_.get_first_trans(state);
    
    if (first_trans == 0) {
        set_next_(index, 0); // to the terminal state
        return;
    }
    
    { // Set next of offset to state's second if needed.
        auto it = state_map_.find(state);
        if (it != state_map_.end()) {
            // already visited state
            set_target_state_(index, it->second);
            return;
        }
    }
    
    const auto next = find_next_(first_trans);
    if (offset_(next) >= bytes_.size()) {
        expand_();
    }
    
    set_target_state_(index, next);
    state_map_.insert(std::make_pair(state, next));
    set_used_next_(next);
    
    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
        const auto symbol = orig_fsa_.get_trans_symbol(trans);
        const auto child_index = next ^ symbol;
        
        freeze_state_(child_index);
        set_check_(child_index, symbol);
        
        if (orig_fsa_.is_final_trans(trans)) {
            set_final_(child_index);
        }
    }
    
    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
        const auto symbol = orig_fsa_.get_trans_symbol(trans);
        arrange_(orig_fsa_.get_target_state(trans), next ^ symbol);
    }
}

// so-called XCHECK
size_t ArrayDACFSABuilder::find_next_(size_t first_trans) const {
    const auto symbol = orig_fsa_.get_trans_symbol(first_trans);
    
    if (unfrozen_head_ != 0) {
        auto unfrozen_index = unfrozen_head_;
        do {
            const auto next = unfrozen_index ^symbol; // TODO: omit index_()
            if (check_next_(next, first_trans)) {
                return next;
            }
            unfrozen_index = get_succ_(unfrozen_index);
        } while (unfrozen_index != unfrozen_head_);
    }
    
    return index_(bytes_.size()) ^ symbol;
}

bool ArrayDACFSABuilder::check_next_(size_t next, size_t trans) const {
    if (is_used_next_(next)) {
        return false;
    }
    
    do {
        const auto index = next ^ orig_fsa_.get_trans_symbol(trans);
        if (is_frozen_(index)) {
            return false;
        }
        trans = orig_fsa_.get_next_trans(trans);
    } while (trans != 0);
    
    return true;
}


//
//  ArrayFSABuilder.cpp
//
//  Created by 松本拓真 on 2017/10/18.
//

#include "array_fsa/ArrayFSABuilder.hpp"

#include "array_fsa/PlainFSA.hpp"
#include "array_fsa/FSA.hpp"
#include "sim_ds/Log.hpp"
#include "sim_ds/Calc.hpp"

using namespace array_fsa;

template <class T>
void ArrayFSABuilder::showInBox(ArrayFSABuilder &builder, T &fsa) {
    auto tab = "\t";
    for (auto i = 0; i < 0x100; i++) {
        auto bn = builder.get_next_(i);
        auto fn = fsa.next(i);
        if (bn != fn) {
            std::cout << i << tab << builder.is_final_(i) << tab << bn << tab << builder.get_check_(i) << std::endl;
            std::cout << i << tab << fsa.isFinal(i) << tab << fn << tab << fsa.check(i) << std::endl;
            sim_ds::Log::showAsBinary(builder.get_next_(i), 4);
            sim_ds::Log::showAsBinary(fsa.next(i), 4);
            std::cout << std::endl;
        }
    }
}

template void ArrayFSABuilder::showInBox(ArrayFSABuilder&, OriginalFSA&);
template void ArrayFSABuilder::showInBox(ArrayFSABuilder&, DacFSA&);

// MARK: - public

void ArrayFSABuilder::showMapping(bool show_density) {
    auto tab = "\t";
    
    std::vector<size_t> next_map;
    next_map.resize(4, 0);
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
        
        auto next = get_next_(i);
        auto size = 0;
        while (next >> (8 * ++size - 1));
        next_map[size - 1]++;
    }
    
    std::cout << "Next size mapping" << std::endl;
    std::cout << "num_elems " << num_elems_() << std::endl;
    std::cout << "\t1\t2\t3\t4\tbyte size" << std::endl;
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

void ArrayFSABuilder::freeze_state_(size_t index) {
    assert(!is_frozen_(index));
    
    set_frozen_(index, true);
    
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

void ArrayFSABuilder::close_block_(size_t begin) {
    const auto end = begin + kBlockSize;
    
    if (unfrozen_head_ == 0 || unfrozen_head_ >= end) {
        return;
    }
    for (auto i = begin; i < end; i++) {
        if (is_frozen_(i)) {
            continue;
        }
        freeze_state_(i);
        set_frozen_(i, false);
    }
}

// so-called XCHECK
size_t ArrayFSABuilder::find_next_(size_t first_trans) const {
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

bool ArrayFSABuilder::check_next_(size_t next, size_t trans) const {
    if (is_used_next_(next)) {
        return false;
    }
    
    do {
        const auto index = next ^orig_fsa_.get_trans_symbol(trans);
        if (is_frozen_(index)) {
            return false;
        }
        trans = orig_fsa_.get_next_trans(trans);
    } while (trans != 0);
    
    return true;
}

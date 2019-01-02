//
// Created by Kampersanda on 2017/05/26.
//

#ifndef Dawg_Builder_hpp
#define Dawg_Builder_hpp

#include "basic.hpp"

#include "PlainFSA.hpp"
#include <unordered_map>

#include "sim_ds/log.hpp"

namespace csd_automata {

class DawgBuilder {
public:
    DawgBuilder(const PlainFSA& src_fsa) : src_fsa_(src_fsa) {}
    
    DawgBuilder(const DawgBuilder&) = delete;
    DawgBuilder& operator=(const DawgBuilder&) = delete;
    
    static constexpr size_t kAddrSize = 4;
    static constexpr size_t kElemSize = 1 + kAddrSize * 4 + 3;

    static constexpr size_t kBlockSize = 0x100;
    static constexpr size_t kFreeBytes = 0x10 * kBlockSize * kElemSize; // like darts-clone
    
    void Build() {
        bytes_.reserve(static_cast<size_t>(src_fsa_.get_num_trans() * 1.1 * kElemSize));
        
        ExpandBlock_();
        FreezeTrans_(0);
        set_final_(0, true);
        set_used_next_(0, true);
        
        Arrange_(src_fsa_.get_root_state(), 0);
        
        // reset
        state_map_ = std::unordered_map<size_t, size_t>();
    }
    
    bool is_final_(size_t index) const {
        return static_cast<bool>(bytes_[offset_(index)] & 1);
    }
    bool is_frozen_(size_t index) const {
        return static_cast<bool>(bytes_[offset_(index)] & 2);
    }
    bool is_used_next_(size_t index) const {
        return static_cast<bool>(bytes_[offset_(index)] & 4);
    }
    size_t get_target_state_(size_t index) const {
        return index ^ get_next_(index);
    }
    size_t get_next_(size_t index) const {
        return get_address_(offset_(index) + 1);
    }
    virtual uint8_t get_check_(size_t index) const {
        return bytes_[offset_(index) + 1 + kAddrSize];
    }
    
    size_t num_elements_() const {
        return bytes_.size() / kElemSize;
    }
    
    template <class T>
    void CheckEquivalence(T &fsa);
    
    void ShowMapping(bool show_density);
    
protected:
    const PlainFSA& src_fsa_;
    
    std::vector<uint8_t> bytes_;
    std::unordered_map<size_t, size_t> state_map_;
    size_t unfrozen_head_ = 0;
    
    // MARK: of array
    size_t index_(size_t offset) const {
        return offset / kElemSize;
    }
    // MARK: of codes
    size_t offset_(size_t index) const {
        return index * kElemSize;
    }
    
    size_t get_address_(size_t offset) const {
        size_t v = 0;
        for (auto i = 0; i < kAddrSize; i++)
            v |= bytes_[offset + i] << (8 * i);
        return v;
    }
    
    // MARK: Getters
    size_t get_succ_(size_t index) const {
        return get_address_(offset_(index) + 1) ^ index;
    }
    size_t get_pred_(size_t index) const {
        return get_address_(offset_(index) + 1 + kAddrSize) ^ index;
    }
    
    // MARK: Setters
    void set_final_(size_t index, bool is_final) {
        auto offset = offset_(index);
        if (is_final) { bytes_[offset] |= 1; }
        else { bytes_[offset] &= ~1; }
    }
    void set_frozen_(size_t index, bool is_frozen) {
        auto offset = offset_(index);
        if (is_frozen) { bytes_[offset] |= 2; }
        else { bytes_[offset] &= ~2; }
    }
    void set_used_next_(size_t index, bool is_used_next) {
        auto offset = offset_(index);
        if (is_used_next) { bytes_[offset] |= 4; }
        else { bytes_[offset] &= ~4; }
    }
    void set_next_(size_t index, size_t next) {
        std::memcpy(&bytes_[offset_(index) + 1], &next, kAddrSize);
    }
    virtual void set_check_(size_t index, uint8_t check) {
        bytes_[offset_(index) + 1 + kAddrSize] = check;
    }
    void set_succ_(size_t index, size_t succ) {
        auto v = index ^ succ;
        std::memcpy(&bytes_[offset_(index) + 1], &v, kAddrSize);
    }
    void set_pred_(size_t index, size_t pred) {
        auto v = index ^ pred;
        std::memcpy(&bytes_[offset_(index) + 1 + kAddrSize], &v, kAddrSize);
    }
    
    // MARK: methods
    
    void ExpandBlock_();
    void FreezeTrans_(size_t index);
    void CloseBlock_(size_t begin);
    
    // Recusive function
    virtual void Arrange_(size_t state, size_t index);
    
    // so-called XCHECK
    size_t FindNext_(size_t first_trans) const;
    bool CheckNext_(size_t next, size_t trans) const;
    
};


// MARK: - public

template <class T>
inline void DawgBuilder::CheckEquivalence(T &fsa) {
    auto tab = "\t";
    for (auto i = 0; i < num_elements_(); i++) {
        if (!is_frozen_(i)) continue;
        auto bn = get_next_(i);
        auto bc = get_check_(i);
        auto bf = is_final_(i);
        auto fn = fsa.next(i);
        auto fc = fsa.check(i);
        auto ff = fsa.isFinal(i);
        if (bn == fn && bc == fc && bf == ff)
            continue;
        
        using std::cout, std::endl;
        cout << i << "] builder" << tab << "fsa" << endl;
        cout << "next: " << bn << tab << fn << endl;
        cout << "check: " << bc << tab << fc << endl;
        cout << "is-final: " << bf << tab << ff << endl;
//            std::cout << i << tab << isFinal_(i) << tab << bn << tab << bc << std::endl;
//            std::cout << i << tab << fsa.isFinal(i) << tab << fn << tab << fc << std::endl;
        sim_ds::ShowAsBinary(bn, 4);
        sim_ds::ShowAsBinary(fn, 4);
        std::cout << std::endl;
    }
}

inline void DawgBuilder::ShowMapping(bool show_density) {
    auto tab = "\t";
    
    std::vector<size_t> next_map;
    next_map.resize(4, 0);
    std::vector<size_t> dens_map;
    const auto dens_block_size = 0x100;
    dens_map.resize(num_elements_() / dens_block_size, 0);
    
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
    std::cout << "num_elems " << num_elements_() << std::endl;
    std::cout << "\t1\t2\t3\t4\tbyte size" << std::endl;
    for (auto num: next_map) {
        auto per_num = int(double(num) / num_elements_() * 100);
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

void DawgBuilder::ExpandBlock_() {
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
        CloseBlock_(begin - index_(kFreeBytes));
    }
}

void DawgBuilder::FreezeTrans_(size_t index) {
    assert(!is_frozen_(index));
    
    set_frozen_(index, true);
    
    const auto succ = get_succ_(index);
    const auto pred = get_pred_(index);
    
    // unlink
    set_succ_(pred, succ);
    set_pred_(succ, pred);
    
    // set succ and pred to 0
    set_succ_(index, 0);
    set_pred_(index, 0);
    
    if (index == unfrozen_head_) {
        unfrozen_head_ = succ == index ? 0 : succ;
    }
}

void DawgBuilder::CloseBlock_(size_t begin) {
    const auto end = begin + kBlockSize;
    
    if (unfrozen_head_ == 0 or unfrozen_head_ >= end) {
        return;
    }
    for (auto i = begin; i < end; i++) {
        if (is_frozen_(i)) {
            continue;
        }
        FreezeTrans_(i);
        set_frozen_(i, false);
    }
}

// so-called XCHECK
size_t DawgBuilder::FindNext_(size_t first_trans) const {
    const auto symbol = src_fsa_.get_trans_symbol(first_trans);
    
    if (unfrozen_head_ != 0) {
        auto unfrozen_index = unfrozen_head_;
        do {
            const auto next = unfrozen_index ^ symbol; // TODO: omit index_()
            if (CheckNext_(next, first_trans)) {
                return next;
            }
            unfrozen_index = get_succ_(unfrozen_index);
        } while (unfrozen_index != unfrozen_head_);
    }
    
    return index_(bytes_.size()) ^ symbol;
}

bool DawgBuilder::CheckNext_(size_t next, size_t trans) const {
    if (is_used_next_(next)) {
        return false;
    }
    
    bool rejected = false;
    auto check_next = [&](uint8_t symbol) {
        auto index = next ^ symbol;
        bool is_frozen = is_frozen_(index);
        if (is_frozen) {
            rejected = true;
        }
    };
    src_fsa_.ForAllSymbolInFollowsTrans(trans, &rejected, check_next);
    return !rejected;
}

// Recusive function
inline void DawgBuilder::Arrange_(size_t state, size_t index) {
    const auto first_trans = src_fsa_.get_first_trans(state);
    
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
    
    const auto next = FindNext_(first_trans);
    if (offset_(next) >= bytes_.size()) {
        ExpandBlock_();
    }
    
    set_next_(index, next);
    state_map_.insert(std::make_pair(state, next));
    set_used_next_(next, true);
    
    for (auto trans = first_trans; trans != 0; trans = src_fsa_.get_next_trans(trans)) {
        const auto symbol = src_fsa_.get_trans_symbol(trans);
        const auto child_index = next ^ symbol;
        
        FreezeTrans_(child_index);
        
        set_check_(child_index, symbol);
        
        if (src_fsa_.is_final_trans(trans)) {
            set_final_(child_index, true);
        }
    }
    
    for (auto trans = first_trans; trans != 0; trans = src_fsa_.get_next_trans(trans)) {
        const auto symbol = src_fsa_.get_trans_symbol(trans);
        Arrange_(src_fsa_.get_target_state(trans), next ^ symbol);
    }
}
    
}

#endif //Dawg_Builder_hpp

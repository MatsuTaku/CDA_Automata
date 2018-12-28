//
//  ArrayFSATailBuilder.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#ifndef ArrayFSA_TailBuilder_hpp
#define ArrayFSA_TailBuilder_hpp

#include "basic.hpp"
#include "PlainFSA.hpp"
#include "TailDict.hpp"
#include "sim_ds/log.hpp"

#include "DoubleArrayCFSA.hpp"

namespace csd_automata {
    

class DoubleArrayCFSABuilder {
    static constexpr size_t kAddrSize = 4;
    static constexpr size_t kElemSize = 1 + kAddrSize * 4 + 3;
    
    static constexpr size_t kBlockSize = 0x100;
    static constexpr size_t kFreeBytes = 0x10 * kBlockSize * kElemSize; // like darts-clone
    
    const PlainFSA& src_fsa_;
    
    std::vector<uint8_t> bytes_;
    std::unordered_map<size_t, size_t> state_map_;
    size_t unfrozen_head_ = 0;
    TailDict tail_dict_;
    
public:
    explicit DoubleArrayCFSABuilder(const PlainFSA& src_fsa) : src_fsa_(src_fsa) {}
    
    void Build(bool binary_mode, bool merge_suffix, bool divide_front);
    
    template <class Product>
    void Release(Product& da);
    
    DoubleArrayCFSABuilder(const DoubleArrayCFSABuilder&) = delete;
    DoubleArrayCFSABuilder& operator=(const DoubleArrayCFSABuilder&) = delete;
    
private:
    template <class Fsa>
    void CheckEquivalence(Fsa& fsa);
    
    // MARK: - getter
    
    size_t num_elements_() const {
        return bytes_.size() / kElemSize;
    }
    
    void ShowMapping(bool show_density);
    
    // MARK: of array
    
    size_t index_(size_t offset) const {
        return offset / kElemSize;
    }
    
    size_t block_(size_t index) const {
        return index / kBlockSize;
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
        assert(is_frozen_(index));
        return get_address_(offset_(index) + 1);
    }
    
    bool has_label_(size_t index) const {
        return static_cast<bool>(bytes_[offset_(index)] & 8);
    }
    
    size_t get_label_number_(size_t index) const {
        assert(is_frozen_(index));
        return get_address_(offset_(index) + 1 + kAddrSize);
    }
    
    size_t get_num_words() const {
        return src_fsa_.get_num_words();
    }
    
    size_t get_words_(size_t index) const {
        return get_address_(offset_(index) + 1 + kAddrSize * 2);
    }
    
    size_t get_cum_words_(size_t index) const {
        return get_address_(offset_(index) + 1 + kAddrSize * 3);
    }
    
    bool has_brother_(size_t index) const {
        return static_cast<bool>(bytes_[offset_(index)] & 16);
    }
    
    uint8_t get_brother_(size_t index) const {
        return bytes_[offset_(index) + 1 + kAddrSize * 4];
    }
    
    uint8_t get_eldest_(size_t index) const {
        return bytes_[offset_(index) + 2 + kAddrSize * 4];
    }
    
    uint8_t get_check_(size_t index) const {
        return bytes_[offset_(index) + 3 + kAddrSize * 4];
    }
    
    size_t get_succ_(size_t index) const {
        assert(!is_frozen_(index));
        return get_address_(offset_(index) + 1) ^ index;
    }
    
    size_t get_pred_(size_t index) const {
        assert(!is_frozen_(index));
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
    
    void set_succ_(size_t index, size_t succ) {
        auto v = index ^ succ;
        std::memcpy(&bytes_[offset_(index) + 1], &v, kAddrSize);
    }
    
    void set_pred_(size_t index, size_t pred) {
        auto v = index ^ pred;
        std::memcpy(&bytes_[offset_(index) + 1 + kAddrSize], &v, kAddrSize);
    }
    
    // MARK: for CFSA
    
    void set_has_label_(size_t index) {
        bytes_[offset_(index)] |= 8;
    }
    
    void set_label_index_(size_t index, size_t labelIndex) {
        std::memcpy(&bytes_[offset_(index) + 1 + kAddrSize], &labelIndex, 4);
    }
    
    void set_words_(size_t index, size_t words) {
        std::memcpy(&bytes_[offset_(index) + 1 + kAddrSize * 2], &words, 4);
    }
    
    void set_cum_words_(size_t index, size_t cw) {
        std::memcpy(&bytes_[offset_(index) + 1 + kAddrSize * 3], &cw, 4);
    }
    
    void set_has_brother_(size_t index) {
        bytes_[offset_(index)] |= 16;
    }
    
    void set_brother_(size_t index, uint8_t bro) {
        bytes_[offset_(index) + 1 + kAddrSize * 4] = bro;
    }
    
    void set_eldest_(size_t index, uint8_t eldest) {
        bytes_[offset_(index) + 2 + kAddrSize * 4] = eldest;
    }
    
    void set_check_(size_t index, uint8_t check) {
        bytes_[offset_(index) + 3 + kAddrSize * 4] = check;
    }
    
    // MARK: methods
    
    void ExpandBlock_();
    void FreezeTrans_(size_t index);
    void CloseBlock_(size_t begin);
    
    // Recusive function
    void Arrange_(size_t state, size_t index);
    
    // so-called XCHECK
    size_t FindNext_(size_t first_trans) const;
    bool CheckNext_(size_t next, size_t trans) const;
    
};

    
void DoubleArrayCFSABuilder::Build(bool binary_mode, bool merge_suffix, bool divide_front) {
    ExpandBlock_();
    FreezeTrans_(0);
    set_final_(0, true);
    set_used_next_(0, true);
    
    tail_dict_.Build(src_fsa_, binary_mode, merge_suffix, divide_front);
    Arrange_(src_fsa_.get_root_state(), 0);
    
    // reset
    state_map_ = std::unordered_map<size_t, size_t>();
}

template <class Product>
void DoubleArrayCFSABuilder::Release(Product& dam) {
    const auto num_elems = num_elements_();
    dam.Base::resize(num_elems, get_num_words());
    GetSerializedStringsBuilder(tail_dict_).Release(&dam.strings_map_);
    
    auto num_trans = 0;
    for (size_t i = 0; i < num_elems; i++) {
        if (is_frozen_(i)) {
            num_trans++;
            
            auto is_str_trans = has_label_(i);
            if constexpr (!Product::kCompressNext) {
                dam.Base::set_next(i, get_next_(i));
            } else {
                dam.Base::set_next(i, get_next_(i) ^ i);
            }
            dam.Base::set_is_string(i, is_str_trans);
            dam.Base::set_is_final(i, is_final_(i));
            if (Product::kUnionCheckAndId) {
                if (is_str_trans) {
                    size_t string_index = get_label_number_(i);
                    size_t string_id = Product::kSelectStrId ? dam.strings_map_.id_rank(string_index) : string_index;
                    dam.Base::set_string_id(i, string_id);
                } else
                    dam.Base::set_check(i, get_check_(i));
            } else {
                dam.Base::set_check(i, get_check_(i));
                if (is_str_trans)
                    dam.Base::set_string_id(i, get_label_number_(i));
            }
            
            if constexpr (Product::kSupportAccess)
                dam.Base::set_words(i, get_words_(i));
            if constexpr (Product::kUseCumulativeWords)
                dam.Base::set_cum_words(i, get_cum_words_(i));
            
            if constexpr (Product::kLinkChildren) {
                bool hasBro = has_brother_(i);
                dam.Base::set_has_brother(i, hasBro);
                if (hasBro)
                    dam.Base::set_brother(i, get_brother_(i));
            }
        }
        
        if constexpr (Product::kLinkChildren) {
            bool is_state = is_used_next_(i);
            dam.Base::set_is_state(i, is_state);
            if (is_state)
                dam.Base::set_eldest(i, get_eldest_(i));
        }
    }
    dam.Base::Build();
    
    dam.num_trans_ = num_trans;
    
    CheckEquivalence(dam);
}


template <class Product>
void DoubleArrayCFSABuilder::CheckEquivalence(Product& dam) {
    auto tab = "\t";
    for (size_t i = 0; i < num_elements_(); i++) {
        if (!is_frozen_(i)) continue;
        auto bn = get_next_(i);
        auto bi = has_label_(i);
        auto bc = !bi ? get_check_(i) : (Product::kSelectStrId ? dam.strings_map_.id_rank(get_label_number_(i)) : get_label_number_(i));
        auto fn = dam.target_state_(i);
        auto fi = dam.Base::is_string(i);
        auto fc = !fi ? dam.Base::check(i) : dam.Base::string_id(i);
        if (bn == fn && bc == fc && bi == fi)
            continue;
        
        using std::cout, std::endl;
        cout << i << "] builder" << tab << "fsa" << endl;
        if (bn != fn)
            cout << "next: " << bn << tab << fn << tab << (bn ^ fn) << endl;
        if (bc != fc)
            cout << "check: " << bc << tab << fc << endl;
        cout << "is-str: " << bi << tab << fi << endl;
        if (bi != fi) {
            cout << "accept: " << is_final_(i) << tab << dam.Base::is_final(i) << endl;
            sim_ds::ShowAsBinary(bc, 4);
            sim_ds::ShowAsBinary(fc, 4);
        }
        std::cout << std::endl;
    }
    
}
    

void DoubleArrayCFSABuilder::ShowMapping(bool show_density) {
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

void DoubleArrayCFSABuilder::ExpandBlock_() {
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


void DoubleArrayCFSABuilder::FreezeTrans_(size_t index) {
    assert(!is_frozen_(index));
    
    const auto succ = get_succ_(index);
    const auto pred = get_pred_(index);
    
    // unlink
    set_succ_(pred, succ);
    set_pred_(succ, pred);
    
    // set succ and pred to 0
    set_succ_(index, 0);
    set_pred_(index, 0);
    
    set_frozen_(index, true);
    
    if (index == unfrozen_head_) {
        unfrozen_head_ = succ == index ? 0 : succ;
    }
}


void DoubleArrayCFSABuilder::CloseBlock_(size_t begin) {
    const auto end = begin + kBlockSize;
    
    if (unfrozen_head_ == 0 || unfrozen_head_ >= end) {
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
size_t DoubleArrayCFSABuilder::FindNext_(size_t first_trans) const {
    const auto symbol = src_fsa_.get_trans_symbol(first_trans);
    
    if (unfrozen_head_ != 0) {
        auto unfrozen_index = unfrozen_head_;
        do {
            const auto next = unfrozen_index ^ symbol; // TODO: omit index_()
            if (CheckNext_(next, first_trans)) {
                assert(!is_used_next_(next));
                return next;
            }
            unfrozen_index = get_succ_(unfrozen_index);
        } while (unfrozen_index != unfrozen_head_);
    }
    
    return index_(bytes_.size()) ^ symbol;
}


bool DoubleArrayCFSABuilder::CheckNext_(size_t next, size_t trans) const {
    if (is_used_next_(next)) {
        return false;
    }
    
    bool rejected = false;
    src_fsa_.ForAllSymbolInFollowsTrans(trans, &rejected, [&](uint8_t symbol) {
        auto index = next ^ symbol;
        rejected = is_frozen_(index);
    });
    return !rejected;
}

// Recursive function
void DoubleArrayCFSABuilder::Arrange_(size_t state, size_t index) {
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
    
    // Contain <symbol, trans>
    std::vector<std::pair<uint8_t, size_t>> trans_children;
    
    auto words_counter = 0;
    for (auto trans = first_trans; trans != 0; trans = src_fsa_.get_next_trans(trans)) {
        const auto symbol = src_fsa_.get_trans_symbol(trans);
        const auto child_index = next ^ symbol;
        
        assert(!is_frozen_(child_index));
        FreezeTrans_(child_index);
        assert(is_frozen_(child_index));
        // set check
        set_check_(child_index, symbol);
        assert(get_check_(child_index) == symbol);
        // Set: hasLabel, stringId
        auto trans_index = trans / PlainFSA::kSizeTrans;
        auto label_trans = trans;
        if (tail_dict_.is_label_source(trans_index)) {
            set_has_label_(child_index);
            set_label_index_(child_index, tail_dict_.start_pos(trans_index));
            
            while (src_fsa_.is_straight_state(label_trans)) {
                label_trans = src_fsa_.get_target_state(label_trans);
            }
        }
        // set is_final
        set_final_(child_index, src_fsa_.is_final_trans(label_trans));
        // set word
        const auto words = src_fsa_.get_words_trans(label_trans);
        set_words_(child_index, words);
        set_cum_words_(child_index, words_counter);
        words_counter += words;
        
        // Prepare to transition next node
        trans_children.push_back(std::make_pair(symbol, label_trans));
    }
    
    set_eldest_(next, trans_children.front().first);
    for (auto i = 0; i < trans_children.size() - 1; i++) {
        auto child_index = next ^ trans_children[i].first;
        set_has_brother_(child_index);
        set_brother_(child_index, trans_children[i + 1].first);
    }
    
    // Transition next node
    for (auto& trans_child : trans_children) {
        Arrange_(src_fsa_.get_target_state(trans_child.second), next ^ trans_child.first);
    }
}
    
}

#endif /* ArrayFSA_TailBuilder_hpp */

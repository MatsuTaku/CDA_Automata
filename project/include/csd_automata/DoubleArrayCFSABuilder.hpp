//
//  ArrayFSATailBuilder.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#ifndef ArrayFSA_TailBuilder_hpp
#define ArrayFSA_TailBuilder_hpp

#include "DoubleArrayFSABuilder.hpp"
#include "TailDictBuilder.hpp"
#include "sim_ds/log.hpp"

#include "DoubleArrayCFSA.hpp"

namespace csd_automata {
    
class DoubleArrayCFSABuilder : public DoubleArrayFSABuilder {
public:
    using Super = DoubleArrayFSABuilder;
    
    explicit DoubleArrayCFSABuilder(const PlainFSA &srcFsa) : DoubleArrayFSABuilder(srcFsa) {}
    
    void Build(bool binaryMode, bool merge_suffix, bool divide_front) {
        TailDictBuilder::Build(str_dict_, src_fsa_, binaryMode, merge_suffix, divide_front);
        DoubleArrayFSABuilder::Build();
    }
    
    template <class Product>
    void Release(Product& da);
    
    // MARK: - getter
    
    bool has_label_(size_t index) const {
        return static_cast<bool>(bytes_[offset_(index)] & 8);
    }
    
    size_t get_label_number_(size_t index) const {
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
    
    uint8_t get_check_(size_t index) const override {
        return bytes_[offset_(index) + 3 + kAddrSize * 4];
    }
    
    template <class T>
    void CheckEquivalence(T &fsa);
    
    // MARK: Copy guard
    
    ~DoubleArrayCFSABuilder() = default;
    
    DoubleArrayCFSABuilder(const DoubleArrayCFSABuilder&) = delete;
    DoubleArrayCFSABuilder& operator=(const DoubleArrayCFSABuilder&) = delete;
    
    DoubleArrayCFSABuilder(DoubleArrayCFSABuilder&&) = default;
    DoubleArrayCFSABuilder& operator=(DoubleArrayCFSABuilder&&) = default;
    
private:
    TailDict str_dict_;
    
    // MARK: Setter
    
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
    
    void set_check_(size_t index, uint8_t check) override {
        bytes_[offset_(index) + 3 + kAddrSize * 4] = check;
    }
    
    void Arrange_(size_t state, size_t index) override;
    
};


template <class Product>
void DoubleArrayCFSABuilder::Release(Product& da) {
    const auto numElems = num_elements_();
    da.base_.resize(numElems, get_num_words());
    GetSerializedStringsBuilder(str_dict_).Release(&da.strings_map_);
    
    auto numTrans = 0;
    for (auto i = 0; i < numElems; i++) {
        if (is_frozen_(i)) {
            numTrans++;
            
            auto is_str_trans = has_label_(i);
            da.base_.set_next(i, get_next_(i));
            da.base_.set_is_string(i, is_str_trans);
            da.base_.set_is_final(i, is_final_(i));
            if (Product::kUnionCheckAndId) {
                if (is_str_trans)
                    da.base_.set_string_id(i, get_label_number_(i));
                else
                    da.base_.set_check(i, get_check_(i));
            } else {
                da.base_.set_check(i, get_check_(i));
                if (is_str_trans)
                    da.base_.set_string_id(i, get_label_number_(i));
            }
            
            if constexpr (Product::kSupportAccess)
                da.base_.set_words(i, get_words_(i));
            if constexpr (Product::kUseCumulativeWords)
                da.base_.set_cum_words(i, get_cum_words_(i));
            
            if constexpr (Product::kLinkChildren) {
                bool hasBro = has_brother_(i);
                da.base_.set_has_brother(i, hasBro);
                if (hasBro)
                    da.base_.set_brother(i, get_brother_(i));
            }
        }
        
        if constexpr (Product::kLinkChildren) {
            bool isNode = is_used_next_(i);
            da.base_.set_is_node(i, isNode);
            if (isNode)
                da.base_.set_eldest(i, get_eldest_(i));
        }
    }
    da.base_.Build();
    da.num_trans_ = numTrans;
    
    CheckEquivalence(da);
}


template <class T>
inline void DoubleArrayCFSABuilder::CheckEquivalence(T &fsa) {
    auto tab = "\t";
    for (auto i = 0; i < num_elements_(); i++) {
        if (!is_frozen_(i)) continue;
        auto bn = get_next_(i);
        auto bi = has_label_(i);
        auto bc = !bi ? get_check_(i) : get_label_number_(i);
        auto fn = fsa.base_.next(i);
        auto fi = fsa.base_.is_string(i);
        auto fc = !fi ? fsa.base_.check(i) : fsa.base_.string_id(i);
        if (bn == fn && bc == fc && bi == fi)
            continue;
        
        using std::cout, std::endl;
        cout << i << "] builder" << tab << "fsa" << endl;
        cout << "next: " << bn << tab << fn << endl;
        cout << "check: " << bc << tab << fc << endl;
        cout << "is-str: " << bi << tab << fi << endl;
        cout << "accept: " << is_final_(i) << tab << fsa.base_.is_final(i) << endl;
        if (bi || fi) {
            sim_ds::log::ShowAsBinary(bc, 4);
            sim_ds::log::ShowAsBinary(fc, 4);
        }
        std::cout << std::endl;
    }
    
}


// MARK: - private

// Recursive function
inline void DoubleArrayCFSABuilder::Arrange_(size_t state, size_t index) {
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
    assert(!is_used_next_(next));
    if (offset_(next) >= bytes_.size()) {
        ExpandBlock_();
    }
    
    set_next_(index, next);
    state_map_.insert(std::make_pair(state, next));
    set_used_next_(next, true);
    
    // Contain <symbol, trans>
    std::vector<std::pair<uint8_t, size_t>> next_transes;
    
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
        if (str_dict_.is_label_source(trans_index)) {
            set_has_label_(child_index);
            set_label_index_(child_index, str_dict_.start_pos(trans_index));
            
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
        next_transes.push_back(std::make_pair(symbol, label_trans));
    }
    
    set_eldest_(next, next_transes.front().first);
    for (auto i = 0; i < next_transes.size() - 1; i++) {
        auto childIndex = next ^ next_transes[i].first;
        set_has_brother_(childIndex);
        set_brother_(childIndex, next_transes[i + 1].first);
    }
    
    // Transition next node
    for (auto& next_trans : next_transes) {
        Arrange_(src_fsa_.get_target_state(next_trans.second), next ^ next_trans.first);
    }
}
    
}

#endif /* ArrayFSA_TailBuilder_hpp */

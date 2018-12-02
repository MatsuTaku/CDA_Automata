//
//  ArrayFSATailBuilder.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#ifndef ArrayFSA_TailBuilder_hpp
#define ArrayFSA_TailBuilder_hpp

#include "DoubleArrayFSABuilder.hpp"
#include "StringDictBuilder.hpp"
#include "sim_ds/log.hpp"

#include "DoubleArrayCFSA.hpp"

namespace csd_automata {
    
class DoubleArrayCFSABuilder : public DoubleArrayFSABuilder {
private:
    StringDict str_dict_;
    
public:
    explicit DoubleArrayCFSABuilder(const PlainFSA &srcFsa) : DoubleArrayFSABuilder(srcFsa) {}
    
    void build(bool binaryMode, bool mergeSuffix = false) {
        StringDictBuilder::Build(str_dict_, src_fsa_, binaryMode, mergeSuffix);
        DoubleArrayFSABuilder::build();
    }
    
    template <class DAM_TYPE>
    void release(DAM_TYPE& da);
    
    // MARK: - getter
    
    bool hasLabel_(size_t index) const {
        return static_cast<bool>(bytes_[offset_(index)] & 8);
    }
    
    size_t getLabelNumber_(size_t index) const {
        return getAddress_(offset_(index) + 1 + kAddrSize);
    }
    
    size_t getNumWords() const {
        return src_fsa_.get_num_words();
    }
    
    size_t getStore_(size_t index) const {
        return getAddress_(offset_(index) + 1 + kAddrSize * 2);
    }
    
    size_t getAccStore_(size_t index) const {
        return getAddress_(offset_(index) + 1 + kAddrSize * 3);
    }
    
    bool hasBrother_(size_t index) const {
        return static_cast<bool>(bytes_[offset_(index)] & 16);
    }
    
    uint8_t getBrother_(size_t index) const {
        return bytes_[offset_(index) + 1 + kAddrSize * 4];
    }
    
    uint8_t getEldest_(size_t index) const {
        return bytes_[offset_(index) + 2 + kAddrSize * 4];
    }
    
    template <class T>
    void showCompareWith(T &fsa);
    
    // MARK: Copy guard
    
    ~DoubleArrayCFSABuilder() = default;
    
    DoubleArrayCFSABuilder(const DoubleArrayCFSABuilder&) = delete;
    DoubleArrayCFSABuilder& operator=(const DoubleArrayCFSABuilder&) = delete;
    
    DoubleArrayCFSABuilder(DoubleArrayCFSABuilder&&) = default;
    DoubleArrayCFSABuilder& operator=(DoubleArrayCFSABuilder&&) = default;
    
private:
    // MARK: Setter
    
    void setHasLabel_(size_t index) {
        bytes_[offset_(index)] |= 8;
    }
    
    void setLabelIndex_(size_t index, size_t labelIndex) {
        std::memcpy(&bytes_[offset_(index) + 1 + kAddrSize], &labelIndex, 4);
    }
    
    void setWords_(size_t index, size_t words) {
        std::memcpy(&bytes_[offset_(index) + 1 + kAddrSize * 2], &words, 4);
    }
    
    void setCumWords_(size_t index, size_t cw) {
        std::memcpy(&bytes_[offset_(index) + 1 + kAddrSize * 3], &cw, 4);
    }
    
    void setHasBrother_(size_t index) {
        bytes_[offset_(index)] |= 16;
    }
    
    void setBrother_(size_t index, uint8_t bro) {
        bytes_[offset_(index) + 1 + kAddrSize * 4] = bro;
    }
    
    void setEldest_(size_t index, uint8_t eldest) {
        bytes_[offset_(index) + 2 + kAddrSize * 4] = eldest;
    }
    
    void arrange_(size_t state, size_t index) override;
    
};


template <class Product>
void DoubleArrayCFSABuilder::release(Product& da) {
    const auto numElems = numElems_();
    da.resize(numElems, getNumWords());
    GetSerializedStringsBuilder(str_dict_).Release(da.serialized_strings_);
    
    auto numTrans = 0;
    for (auto i = 0; i < numElems; i++) {
        if (isFrozen_(i)) {
            numTrans++;
            
            auto isStrTrans = hasLabel_(i);
            da.set_next(i, getNext_(i));
            da.set_is_string_trans(i, isStrTrans);
            da.set_is_final(i, isFinal_(i));
            da.set_check(i, getCheck_(i));
            if (isStrTrans)
                da.set_string_id(i, getLabelNumber_(i));
            else
                da.set_check(i, getCheck_(i));
            
            if constexpr (Product::kSupportAccess)
                da.set_words(i, getStore_(i));
            if constexpr (Product::kUseCumulativeWords)
                da.set_cum_words(i, getAccStore_(i));
            
            if constexpr (Product::kLinkChildren) {
                bool hasBro = hasBrother_(i);
                da.set_has_brother(i, hasBro);
                if (hasBro)
                    da.set_brother(i, getBrother_(i));
            }
        }
        
        if constexpr (Product::kLinkChildren) {
            bool isNode = isUsedNext_(i);
            da.set_is_node(i, isNode);
            if (isNode)
                da.set_eldest(i, getEldest_(i));
        }
    }
    da.BuildBitVector();
    da.set_num_trans(numTrans);
    
    showCompareWith(da);
}


template <class T>
inline void DoubleArrayCFSABuilder::showCompareWith(T &fsa) {
    auto tab = "\t";
    for (auto i = 0; i < numElems_(); i++) {
        if (!isFrozen_(i)) continue;
        auto bn = getNext_(i);
        auto bi = hasLabel_(i);
        auto bc = !bi ? getCheck_(i) : getLabelNumber_(i);
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
        cout << "accept: " << isFinal_(i) << tab << fsa.base_.is_final(i) << endl;
        if (bi || fi) {
            sim_ds::log::ShowAsBinary(bc, 4);
            sim_ds::log::ShowAsBinary(fc, 4);
        }
        std::cout << std::endl;
    }
    
}


// MARK: - private

// Recursive function
inline void DoubleArrayCFSABuilder::arrange_(size_t state, size_t index) {
    const auto first_trans = src_fsa_.get_first_trans(state);
    
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
        expandBlock_();
    }
    
    setNext_(index, next);
    state_map_.insert(std::make_pair(state, next));
    setUsedNext_(next, true);
    
    // Contain <symbol, trans>
    std::vector<std::pair<uint8_t, size_t>> nextTranses;
    
    auto storesCounter = 0;
    for (auto trans = first_trans; trans != 0; trans = src_fsa_.get_next_trans(trans)) {
        const auto symbol = src_fsa_.get_trans_symbol(trans);
        const auto child_index = next ^ symbol;
        
        freezeState_(child_index);
        
        // Set: hasLabel, check or stringId
        auto transIndex = trans / PlainFSA::kSizeTrans;
        auto labelTrans = trans;
        if (str_dict_.is_label_source(transIndex)) {
            setHasLabel_(child_index);
            setLabelIndex_(child_index, str_dict_.start_pos(transIndex));
            
            str_dict_.TraceOnLabel(transIndex);
            while (!str_dict_.is_end_label() && src_fsa_.is_straight_state(labelTrans)) {
                labelTrans = src_fsa_.get_target_state(labelTrans);
                str_dict_.pos_to_next();
            }
        } else {
            setCheck_(child_index, symbol);
        }
        
        // set isFinal
        setFinal_(child_index, src_fsa_.is_final_trans(labelTrans));
        
        // set store
        const auto store = src_fsa_.get_store_trans(labelTrans);
        setWords_(child_index, store);
        setCumWords_(child_index, storesCounter);
        storesCounter += store;
        
        // Prepare to transition next node
        nextTranses.push_back(std::make_pair(symbol, labelTrans));
    }
    
    setEldest_(next, nextTranses.front().first);
    for (auto i = 0; i < nextTranses.size() - 1; i++) {
        auto childIndex = next ^ nextTranses[i].first;
        setHasBrother_(childIndex);
        setBrother_(childIndex, nextTranses[i + 1].first);
    }
    
    // Transition next node
    for (auto &nextTrans : nextTranses) {
        arrange_(src_fsa_.get_target_state(nextTrans.second), next ^ nextTrans.first);
    }
}
    
}

#endif /* ArrayFSA_TailBuilder_hpp */

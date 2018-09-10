//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_ARRAYFSABUILDER_HPP
#define ARRAY_FSA_ARRAYFSABUILDER_HPP

#include "basic.hpp"

#include "PlainFSA.hpp"
#include <unordered_map>

#include "sim_ds/Log.hpp"

namespace array_fsa {
    
    class ArrayFSABuilder {
    public:
        ArrayFSABuilder(const PlainFSA &orig_fsa) : orig_fsa_(orig_fsa) {}
        
        virtual ~ArrayFSABuilder() = default;
        
        ArrayFSABuilder(const ArrayFSABuilder&) = delete;
        ArrayFSABuilder& operator=(const ArrayFSABuilder&) = delete;
        
    public:
        static constexpr size_t kAddrSize = 4;
        static constexpr size_t kElemSize = 1 + kAddrSize * 4 + 2;
        
        void build();
        
        bool isFinal_(size_t index) const {
            return (bytes_[offset_(index)] & 1) == 1;
        }
        bool isFrozen_(size_t index) const {
            return (bytes_[offset_(index)] & 2) == 2;
        }
        bool isUsedNext_(size_t index) const {
            return (bytes_[offset_(index)] & 4) == 4;
        }
        size_t getTargetState_(size_t index) const {
            return index ^ getNext_(index);
        }
        size_t getNext_(size_t index) const {
            return getAddress_(offset_(index) + 1);
        }
        uint8_t getCheck_(size_t index) const {
            return bytes_[offset_(index) + 1 + kAddrSize];
        }
        
        size_t numElems_() const {
            return bytes_.size() / kElemSize;
        }
        
        template <class T>
        void showCompareWith(T &fsa);
        
        void showMapping(bool show_density);
        
    protected:
        static constexpr size_t kBlockSize = 0x100;
        static constexpr size_t kFreeBytes = 0x10 * kBlockSize * kElemSize; // like darts-clone
        
        const PlainFSA &orig_fsa_;
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
        
        size_t getAddress_(size_t offset) const {
            size_t v = 0;
            for (auto i = 0; i < kAddrSize; i++)
                v |= bytes_[offset + i] << (8 * i);
            return v;
        }
        
        // MARK: Getters
        size_t getSucc_(size_t index) const {
            return getAddress_(offset_(index) + 1) ^ index;
        }
        size_t getPred_(size_t index) const {
            return getAddress_(offset_(index) + 1 + kAddrSize) ^ index;
        }
        
        // MARK: Setters
        void setFinal_(size_t index, bool is_final) {
            auto offset = offset_(index);
            if (is_final) { bytes_[offset] |= 1; }
            else { bytes_[offset] &= ~1; }
        }
        void setFrozen_(size_t index, bool is_frozen) {
            auto offset = offset_(index);
            if (is_frozen) { bytes_[offset] |= 2; }
            else { bytes_[offset] &= ~2; }
        }
        void setUsedNext_(size_t index, bool is_used_next) {
            auto offset = offset_(index);
            if (is_used_next) { bytes_[offset] |= 4; }
            else { bytes_[offset] &= ~4; }
        }
        void setFinalAndUsedNext_(size_t index) {
            bytes_[offset_(index)] |= 5;
        }
        void setNext_(size_t index, size_t next) {
            auto relative_next = index ^ next;
            std::memcpy(&bytes_[offset_(index) + 1], &relative_next, kAddrSize);
        }
        void setCheck_(size_t index, uint8_t check) {
            bytes_[offset_(index) + 1 + kAddrSize] = check;
        }
        void setSucc_(size_t index, size_t succ) {
            auto v = index ^ succ;
            std::memcpy(&bytes_[offset_(index) + 1], &v, kAddrSize);
        }
        void setPred_(size_t index, size_t pred) {
            auto v = index ^ pred;
            std::memcpy(&bytes_[offset_(index) + 1 + kAddrSize], &v, kAddrSize);
        }
        
        // MARK: methods
        
        void expand_();
        void freezeState_(size_t index);
        void closeBlock_(size_t begin);
        
        // Recusive function
        virtual void arrange_(size_t state, size_t index);
        
        // so-called XCHECK
        size_t findNext_(size_t first_trans) const;
        bool checkNext_(size_t next, size_t trans) const;
        
    };
    
    
    // MARK: - public
    
    inline void ArrayFSABuilder::build() {
        bytes_.reserve(static_cast<size_t>(orig_fsa_.get_num_trans() * 1.1 * kElemSize));
        
        expand_();
        freezeState_(0);
        setFinalAndUsedNext_(0);
        
        arrange_(orig_fsa_.get_root_state(), 0);
        
        // reset
        state_map_ = std::unordered_map<size_t, size_t>();
    }
    
    template <class T>
    inline void ArrayFSABuilder::showCompareWith(T &fsa) {
        auto tab = "\t";
        for (auto i = 0; i < numElems_(); i++) {
            if (!isFrozen_(i)) continue;
            auto bn = getNext_(i);
            auto bc = getCheck_(i);
            auto bf = isFinal_(i);
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
            sim_ds::Log::showAsBinary(bn, 4);
            sim_ds::Log::showAsBinary(fn, 4);
            std::cout << std::endl;
        }
    }
    
    inline void ArrayFSABuilder::showMapping(bool show_density) {
        auto tab = "\t";
        
        std::vector<size_t> next_map;
        next_map.resize(4, 0);
        std::vector<size_t> dens_map;
        const auto dens_block_size = 0x100;
        dens_map.resize(numElems_() / dens_block_size, 0);
        
        for (size_t i = 0, num_node = 0; i < index_(bytes_.size()); i++) {
            if (!isFrozen_(i)) {
                continue;
            }
            
            num_node++;
            if ((i + 1) % dens_block_size == 0) {
                dens_map[(i + 1) / dens_block_size - 1] = double(num_node) / dens_block_size * 1000;
                num_node = 0;
            }
            
            auto next = getNext_(i);
            auto size = 0;
            while (next >> (8 * ++size - 1));
            next_map[size - 1]++;
        }
        
        std::cout << "Next size mapping" << std::endl;
        std::cout << "num_elems " << numElems_() << std::endl;
        std::cout << "\t1\t2\t3\t4\tbyte size" << std::endl;
        for (auto num: next_map) {
            auto per_num = int(double(num) / numElems_() * 100);
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
    
}

#endif //ARRAY_FSA_ARRAYFSABUILDER_HPP

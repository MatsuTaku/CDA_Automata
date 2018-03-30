//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_ARRAYFSABUILDER_HPP
#define ARRAY_FSA_ARRAYFSABUILDER_HPP

#include "basic.hpp"
#include <unordered_map>

#include "PlainFSA.hpp"

namespace array_fsa {
    
    template <bool DAC, class CODES>
    class FSA;
    
    class ArrayFSABuilder {
    public:
        static constexpr size_t kAddrSize = 4;
        static constexpr size_t kElemSize = 1 + kAddrSize * 2;
        
        template <class T>
        static T build(const PlainFSA &origFsa) {
            ArrayFSABuilder builder(origFsa);
            builder.build_();
            
            T newFsa;
            
            const auto numElem = builder.num_elems_();
            newFsa.setNumElement(numElem);
            
            auto numTrans = 0;
            for (auto i = 0; i < numElem; i++) {
                newFsa.setCheck(i, builder.get_check_(i));
                newFsa.setNextAndIsFinal(i, builder.get_next_(i), builder.is_final_(i));
                if (builder.is_frozen_(i))
                    numTrans++;
            }
            newFsa.setNumTrans(numTrans);
            newFsa.buildBitArray();
            
            //    showInBox(builder, newFsa);
            
            return newFsa;
        }
        
        template <class T>
        static void showInBox(ArrayFSABuilder&, T&);
        
        ArrayFSABuilder(const ArrayFSABuilder&) = delete;
        ArrayFSABuilder& operator=(const ArrayFSABuilder&) = delete;
        
        // MARK: - Addition Matsumoto
        
        void showMapping(bool show_density);
        
    protected:
        static constexpr size_t kBlockSize = 0x100;
        static constexpr size_t kFreeBytes = 0x10 * kBlockSize * kElemSize; // like darts-clone
        
        const PlainFSA &orig_fsa_;
        std::vector<uint8_t> bytes_;
        std::unordered_map<size_t, size_t> state_map_;
        size_t unfrozen_head_ = 0;
        
        explicit ArrayFSABuilder(const PlainFSA &orig_fsa) : orig_fsa_(orig_fsa) {}
        ~ArrayFSABuilder() = default;
        
        // MARK: of array
        size_t index_(size_t offset) const {
            return offset / kElemSize;
        }
        // MARK: of codes
        size_t offset_(size_t index) const {
            return index * kElemSize;
        }
        
        size_t getAddress(size_t offset) const {
            size_t v = 0;
            for (auto i = 0; i < kAddrSize; i++)
                v |= bytes_[offset + i] << (8 * i);
            return v;
        }
        
        size_t num_elems_() const {
            return bytes_.size() / kElemSize;
        }
        
        // MARK: Getters
        bool is_final_(size_t index) const {
            return (bytes_[offset_(index)] & 1) == 1;
        }
        bool is_frozen_(size_t index) const {
            return (bytes_[offset_(index)] & 2) == 2;
        }
        bool is_used_next_(size_t index) const {
            return (bytes_[offset_(index)] & 4) == 4;
        }
        size_t get_target_state_(size_t index) const {
            return index ^ get_next_(index);
        }
        size_t get_next_(size_t index) const {
            return getAddress(offset_(index) + 1);
        }
        uint8_t get_check_(size_t index) const {
            return bytes_[offset_(index) + 1 + kAddrSize];
        }
        size_t get_succ_(size_t index) const {
            return getAddress(offset_(index) + 1) ^ index;
        }
        size_t get_pred_(size_t index) const {
            return getAddress(offset_(index) + 1 + kAddrSize) ^ index;
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
        void set_true_final_and_used_next_(size_t index) {
            bytes_[offset_(index)] |= 5;
        }
        void set_next_(size_t index, size_t next) {
            auto relative_next = index ^ next;
            std::memcpy(&bytes_[offset_(index) + 1], &relative_next, kAddrSize);
        }
        void set_check_(size_t index, uint8_t check) {
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
        
        void build_();
        void expand_();
        void freeze_state_(size_t index);
        void close_block_(size_t begin);
        
        virtual void arrange_(size_t state, size_t index) {
            const auto first_trans = orig_fsa_.get_first_trans(state);
            
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
            
            const auto next = find_next_(first_trans);
            if (offset_(next) >= bytes_.size()) {
                expand_();
            }
            
            set_next_(index, next);
            state_map_.insert(std::make_pair(state, next));
            set_used_next_(next, true);
            
            for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
                const auto symbol = orig_fsa_.get_trans_symbol(trans);
                const auto child_index = next ^ symbol;
                
                freeze_state_(child_index);
                set_check_(child_index, symbol);
                
                if (orig_fsa_.is_final_trans(trans)) {
                    set_final_(child_index, true);
                }
            }
            
            for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
                const auto symbol = orig_fsa_.get_trans_symbol(trans);
                arrange_(orig_fsa_.get_target_state(trans), next ^ symbol);
            }
        }
        
        // so-called XCHECK
        size_t find_next_(size_t first_trans) const;
        bool check_next_(size_t next, size_t trans) const;
        
    };
    
}

#endif //ARRAY_FSA_ARRAYFSABUILDER_HPP

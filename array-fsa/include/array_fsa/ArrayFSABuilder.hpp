//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_ARRAYFSABUILDER_HPP
#define ARRAY_FSA_ARRAYFSABUILDER_HPP

#include "basic.hpp"

#include <unordered_map>

#include "sim_ds/Log.hpp"

namespace array_fsa {
    
    template <bool DAC>
    class FSA;
    
    class PlainFSA;
    
    class ArrayFSABuilder {
    public:
        static constexpr size_t kAddrSize = 4;
        static constexpr size_t kElemSize = 1 + kAddrSize * 2;
        
        template <class T>
        static T build(const PlainFSA &origFsa);
        
        template <class T>
        void showInBox(T &fsa);
        
        void showMapping(bool show_density);
        
        ~ArrayFSABuilder() = default;
        
        ArrayFSABuilder(const ArrayFSABuilder&) = delete;
        ArrayFSABuilder& operator=(const ArrayFSABuilder&) = delete;
        
    protected:
        static constexpr size_t kBlockSize = 0x100;
        static constexpr size_t kFreeBytes = 0x10 * kBlockSize * kElemSize; // like darts-clone
        
        const PlainFSA &orig_fsa_;
        std::vector<uint8_t> bytes_;
        std::unordered_map<size_t, size_t> state_map_;
        size_t unfrozen_head_ = 0;
        
        explicit ArrayFSABuilder(const PlainFSA &orig_fsa) : orig_fsa_(orig_fsa) {}
        
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
        
        size_t numElems_() const {
            return bytes_.size() / kElemSize;
        }
        
        // MARK: Getters
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
        
        void build_();
        void expand_();
        void freezeState_(size_t index);
        void closeBlock_(size_t begin);
        
        // Recusive function
        virtual void arrange_(size_t state, size_t index);
        
        // so-called XCHECK
        size_t findNext_(size_t first_trans) const;
        bool checkNext_(size_t next, size_t trans) const;
        
    };
    
    
    template <class T>
    T ArrayFSABuilder::build(const PlainFSA &origFsa) {
        ArrayFSABuilder builder(origFsa);
        builder.build_();
        
        T newFsa;
        
        const auto numElem = builder.numElems_();
        newFsa.setNumElement(numElem);
        
        auto numTrans = 0;
        for (auto i = 0; i < numElem; i++) {
            newFsa.setCheck(i, builder.getCheck_(i));
            newFsa.setNextAndIsFinal(i, builder.getNext_(i), builder.isFinal_(i));
            if (builder.isFrozen_(i))
                numTrans++;
        }
        newFsa.setNumTrans(numTrans);
        newFsa.buildBitArray();
        
        builder.showInBox(newFsa);
        
        return newFsa;
    }
    
    template <class T>
    void ArrayFSABuilder::showInBox(T &fsa) {
        auto tab = "\t";
        for (auto i = 0; i < 0x100; i++) {
            auto bn = getNext_(i);
            auto fn = fsa.next(i);
            if (bn != fn) {
                std::cout << i << tab << isFinal_(i) << tab << bn << tab << getCheck_(i) << std::endl;
                std::cout << i << tab << fsa.isFinal(i) << tab << fn << tab << fsa.check(i) << std::endl;
                sim_ds::Log::showAsBinary(getNext_(i), 4);
                sim_ds::Log::showAsBinary(fsa.next(i), 4);
                std::cout << std::endl;
            }
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
    
}

#endif //ARRAY_FSA_ARRAYFSABUILDER_HPP

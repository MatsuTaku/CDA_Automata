//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_DAC_FSA_ArrayDACFSA_HPP
#define ARRAY_DAC_FSA_ArrayDACFSA_HPP

#include "basic.hpp"
#include "ArrayDACFSARank.hpp"
#include "DacUnit.hpp"

namespace array_fsa {
    
    class ArrayDACFSABuilder;
    
    class ArrayDACFSA {
        friend class ArrayDACFSABuilder;
        
    public:
        ArrayDACFSA() = default;
        ~ArrayDACFSA() = default;
        
        using Builder = ArrayDACFSABuilder;
        
        ArrayDACFSA(ArrayDACFSA&& rhs) noexcept : ArrayDACFSA() {
            this->swap(rhs);
        }
        ArrayDACFSA& operator=(ArrayDACFSA&& rhs) noexcept {
            this->swap(rhs);
            return *this;
        }
        
        static std::string name() {
            return "ArrayDACFSA";
        }
        
        size_t offset_(size_t trans) const {
            return trans * element_size_;
        }
        
        size_t get_root_state() const {
            return get_next_(0);
        }
        
        size_t get_trans(size_t state, uint8_t symbol) const {
            const auto trans = state ^ symbol;
            auto is_match = get_check_(trans) == symbol;
            return is_match ? trans : 0;
        }
        
        size_t get_target_state(size_t trans) const {
            return trans ^ get_next_(trans);
        }
        
        size_t get_next_(size_t trans) const {
            size_t next = 0;
            std::memcpy(&next, &bytes_[offset_(trans)], 1);
            // TODO: DAC
            for (size_t size = 1, index = trans; size < next_size_; size++) {
                auto &unit = dac_units_[size - 1];
                if (!unit.get(index)) {
                    break;
                }
                next |= unit.getByte(index) << (8 * size);
                index = unit.rank(index) - 1;
            }
            return next;
        }
        
//        bool is_used_DAC_(size_t trans) const {
//            return dac_units_[0].get(trans);
//        }
        
        bool is_final_trans(size_t trans) const {
            return is_final_bits.get(trans);
        }
        
        uint8_t get_check_(size_t trans) const { // == get_trans_symbol
            return bytes_[offset_(trans) + 1];
        }
        
        size_t get_num_trans() const {
            return num_trans_;
        }
        
        size_t get_num_elements() const {
            return bytes_.size() / element_size_;
        }
        
        void set_next(size_t trans, size_t next) {
            size_t mask = 0xff;
            auto firstUnit = next & mask;
            std::memcpy(&bytes_[offset_(trans)], &firstUnit, 1);
            auto nextSize = 0;
            while (next >> (8 * ++nextSize));
//            Rank::show_as_bytes(next, 4);
            for (size_t size = 1, index = trans; size < nextSize; size++) {
                auto &unit = dac_units_[size - 1];
                unit.setBit(index, true);
                auto byte = (next >> (8 * size)) & mask;
//                Rank::show_as_bytes(byte, 1);
                unit.setByte(byte);
                index = unit.numBytes() - 1;
            }
        }
        
        void set_is_final(size_t trans) {
            is_final_bits.set(trans, true);
        }
        
//        void set_used_DAC(size_t trans) {
//            dac_units_[0].setBit(trans, true);
//        }
        
        void set_check(size_t trans, uint8_t check) {
            bytes_[offset_(trans) + 1] = check;
        }
        
        void calc_next_size(size_t num_elems) {
            next_size_ = 0;
            while ((num_elems - 1) >> (8 * ++next_size_));
            dac_units_.resize(next_size_ - 1);
        }
        
        void buildBits() {
            is_final_bits.build();
            for (auto &u : dac_units_) {
                u.build();
            }
        }
        
        void write(std::ostream& os) const {
            write_vec(bytes_, os);
            is_final_bits.write(os);
            write_val(next_size_, os);
            write_val(num_trans_, os);
            for (auto &u : dac_units_) {
                u.write(os);
            }
        }
        
        void read(std::istream& is) {
            bytes_ = read_vec<uint8_t>(is);
            is_final_bits.read(is);
            next_size_ = read_val<size_t>(is);
            element_size_ = 1 + 1;
            num_trans_ = read_val<size_t>(is);
            for (auto i = 0; i < next_size_ - 1; i++) {
                DacUnit unit;
                unit.read(is);
                dac_units_.push_back(unit);
            }
        }
        
        size_t size_in_bytes() const {
            size_t size = 0;
            size += size_vec(bytes_);
            size += sizeof(next_size_);
            size += sizeof(num_trans_);
            return size;
        }
        
        void show_stat(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << get_num_trans() << endl;
            os << "#elems: " << get_num_elements() << endl;
            os << "size:   " << size_in_bytes() << endl;
            os << "size bytes:   " << size_vec(bytes_) << endl;
            os << "size dac:   " << size_vec(dac_units_) << endl;
        }
        
        void swap(ArrayDACFSA& rhs) {
            bytes_.swap(rhs.bytes_);
            std::swap(next_size_, rhs.next_size_);
            std::swap(element_size_, rhs.element_size_);
            std::swap(num_trans_, rhs.num_trans_);
        }
        
        ArrayDACFSA(const ArrayDACFSA&) = delete;
        ArrayDACFSA& operator=(const ArrayDACFSA&) = delete;
        
    private:
        std::vector<uint8_t> bytes_;
        Rank is_final_bits;
        size_t next_size_ = 0;
        size_t element_size_ = 0;
        size_t num_trans_ = 0;
        std::vector<DacUnit> dac_units_;
        
        size_t dac_flow_size_() const {
            return next_size_ - 1;
        }
    };
    
}

#endif //ARRAY_FSA_ArrayDACFSA_HPP

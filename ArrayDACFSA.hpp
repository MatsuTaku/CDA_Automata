//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_DAC_FSA_ArrayDACFSA_HPP
#define ARRAY_DAC_FSA_ArrayDACFSA_HPP

#include "basic.hpp"
#include "ArrayDACFSARank.hpp"

namespace array_fsa {
    
    class ArrayDACFSA {
        friend class ArrayDACFSABuilder;
    public:
        ArrayDACFSA() = default;
        ~ArrayDACFSA() = default;
        
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
            std::memcpy(&next, &bytes_[offset_(trans)], dac_unit_size);
            // TODO: DAC
            if (is_used_DAC_(trans)) {
                size_t nextFlow = get_DAC_flow_(trans);
                next = (nextFlow << (8 * dac_unit_size)) | next;
                
                // TODO: Show
                //                std::cout << "next = " << next << std::endl;
                //                Rank::show_as_bytes(nextFlow, dac_flow_size_());
                //                Rank::show_as_bytes(next, dac_unit_size);
            }
            return next;
        }
        
        bool is_used_DAC_(size_t trans) const {
            return flag_structs_.get_used_dac(trans);
        }
        
        bool is_final_trans(size_t trans) const {
            return flag_structs_.get_is_final(trans);
        }
        
        uint8_t get_check_(size_t trans) const { // == get_trans_symbol
            return bytes_[offset_(trans) + dac_unit_size];
        }
        
        size_t get_num_trans() const {
            return num_trans_;
        }
        
        size_t get_num_elements() const {
            return bytes_.size() / element_size_;
        }
        
        void set_next(size_t trans, size_t next) {
            if (dac_unit_size < sizeof(size_t)) {
                size_t first_unit_mask = (size_t(1) << (8 * dac_unit_size)) - 1;
                auto first_unit_next = next & first_unit_mask;
                std::memcpy(&bytes_[offset_(trans)], &first_unit_next, dac_unit_size);
                auto nextFlow = next >> (8 * dac_unit_size);
                auto needsDAC = nextFlow > 0;
                if (needsDAC) {
                    set_used_DAC(trans);
                    for (auto i = 0; i < dac_flow_size_(); i++) {
                        dac_bytes_.push_back((nextFlow >> (i * 8)) & 0xff);
                    }
                }
                
                // TODO: Test set_next
//                auto union_next = first_unit_next | nextFlow << (8 * dac_unit_size);
//                if (union_next != next) {
//                    std::cout << "setnext error: " << next << "\t" << union_next << std::endl;
//                }
            } else {
                std::memcpy(&bytes_[offset_(trans)], &next, dac_unit_size);
            }
        }
        
        void set_is_final(size_t trans) {
            flag_structs_.set_is_final(trans, true);
        }
        
        void set_used_DAC(size_t trans) {
            flag_structs_.set_used_dac(trans, true);
        }
        
        void set_check(size_t trans, uint8_t check) {
            bytes_[offset_(trans) + dac_unit_size] = check;
        }
        
        void calc_next_size(size_t num_elems) {
            next_size_ = 0;
            while (num_elems >> (8 * ++next_size_));
        }
        
        void write(std::ostream& os) const {
            write_vec(bytes_, os);
            write_vec(dac_bytes_, os);
            write_val(next_size_, os);
            write_val(dac_unit_size, os);
            write_val(num_trans_, os);
            flag_structs_.write(os);
        }
        
        void read(std::istream& is) {
            bytes_ = read_vec<uint8_t>(is);
            dac_bytes_ = read_vec<uint8_t>(is);
            next_size_ = read_val<size_t>(is);
            dac_unit_size = read_val<size_t>(is);
            element_size_ = dac_unit_size + 1;
            num_trans_ = read_val<size_t>(is);
            flag_structs_.read(is);
        }
        
        size_t size_in_bytes() const {
            size_t size = 0;
            size += size_vec(bytes_);
            size += size_vec(dac_bytes_);
            size += sizeof(next_size_);
            size += sizeof(dac_unit_size);
            size += sizeof(num_trans_);
            size += flag_structs_.size_in_bytes();
            return size;
        }
        
        void show_stat(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << get_num_trans() << endl;
            os << "#elems: " << get_num_elements() << endl;
            os << "size:   " << size_in_bytes() << endl;
            os << "size bytes:   " << size_vec(bytes_) << endl;
            os << "size dac:   " << size_vec(dac_bytes_) << endl;
        }
        
        void swap(ArrayDACFSA& rhs) {
            bytes_.swap(rhs.bytes_);
            flag_structs_.swap(rhs.flag_structs_);
            dac_bytes_.swap(rhs.dac_bytes_);
            std::swap(next_size_, rhs.next_size_);
            std::swap(dac_unit_size, rhs.dac_unit_size);
            std::swap(element_size_, rhs.element_size_);
            std::swap(num_trans_, rhs.num_trans_);
        }
        
        ArrayDACFSA(const ArrayDACFSA&) = delete;
        ArrayDACFSA& operator=(const ArrayDACFSA&) = delete;
        
    private:
        std::vector<uint8_t> bytes_;
        ArrayDACFSARank flag_structs_;
        std::vector<uint8_t> dac_bytes_;
        size_t next_size_ = 0;
        size_t dac_unit_size = 0;
        size_t element_size_ = 0;
        size_t num_trans_ = 0;
        
        size_t dac_flow_size_() const {
            return next_size_ - dac_unit_size;
        }
        size_t get_DAC_flow_(size_t trans) const {
            size_t flow = 0;
            std::memcpy(&flow, &dac_bytes_[offset_rank_(trans)], dac_flow_size_());
            return flow;
        }
        
        size_t offset_rank_(size_t trans) const {
            return (rank_(trans) - 1) * dac_flow_size_();
        }
        
        size_t rank_(size_t trans) const {
            return flag_structs_.rank(trans);
        }
    };
    
}

#endif //ARRAY_FSA_ArrayDACFSA_HPP

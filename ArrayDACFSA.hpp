//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_DAC_FSA_ArrayDACFSA_HPP
#define ARRAY_DAC_FSA_ArrayDACFSA_HPP

#include "basic.hpp"
#include "Rank.hpp"

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
        bool is_final_trans(size_t trans) const {
            return (bytes_[offset_(trans)] & 1) == 1;
        }
        
        size_t get_num_trans() const {
            return num_trans_;
        }
        size_t get_num_elements() const {
            return bytes_.size() / element_size_;
        }
        
        void set_is_final(size_t trans, bool is_final) {
            bytes_[offset_(trans)] |= (is_final ? 1 : 0);
        }
        
        void set_needs_DAC(size_t trans) {
            dac_flags_.set(trans);
        }
        
        void set_next(size_t trans, size_t next) {
            auto one_byte_next = next & 0xff;
            std::memcpy(&bytes_[offset_(trans) + 1], &one_byte_next, 1);
            auto nextFlow = next / 0x100;
            auto needsDAC = nextFlow > 0;
            if (needsDAC) {
                set_needs_DAC(trans);
                for (auto i = 0; i < dac_flow_size_(); i++) {
                    dac_bytes_.push_back((nextFlow >> (i * 8)) & 0xff);
                }
            }
        }
        
        void set_check(size_t trans, uint8_t check) {
            bytes_[offset_(trans) + 2] = check;
        }
        
        void calc_next_size(size_t num_elems) {
            next_size_ = 0;
            while (num_elems >> (8 * ++next_size_ - 1));
        }
        
        void write(std::ostream& os) const {
            write_vec(bytes_, os);
            write_vec(dac_bytes_, os);
            write_val(next_size_, os);
            write_val(num_trans_, os);
            dac_flags_.write(os);
        }
        void read(std::istream& is) {
            bytes_ = read_vec<uint8_t>(is);
            dac_bytes_ = read_vec<uint8_t>(is);
            next_size_ = read_val<size_t>(is);
            element_size_ = 3;
            num_trans_ = read_val<size_t>(is);
            dac_flags_.read(is);
        }
        
        size_t size_in_bytes() const {
            return size_vec(bytes_) + size_vec(dac_bytes_) + sizeof(next_size_) + sizeof(num_trans_) + dac_flags_.size_in_bytes();
        }
        
        void show_stat(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << get_num_trans() << endl;
            os << "#elems: " << get_num_elements() << endl;
            os << "size:   " << size_in_bytes() << endl;
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
        Rank dac_flags_;
        std::vector<uint8_t> dac_bytes_;
        size_t next_size_ = 0;
        size_t element_size_ = 0;
        size_t num_trans_ = 0;
        
        size_t dac_flow_size_() const {
            return next_size_ - 1;
        }
        
        size_t get_next_(size_t trans) const { // == get_target_state
            size_t next = 0;
            std::memcpy(&next, &bytes_[offset_(trans) + 1], 1);
            // TODO: DAC
            if (is_used_DAC_(trans)) {
                size_t nextFlow = get_DAC_flow(trans);
                next = nextFlow << 8 | next;
            }
            return next;
        }
        size_t get_DAC_flow(size_t trans) const {
            size_t flow = 0;
            auto offsetRank = offset_rank_(rank_(trans) - 1);
            std::memcpy(&flow, &dac_bytes_[offsetRank], dac_flow_size_());
            return flow;
        }
        uint8_t get_check_(size_t trans) const { // == get_trans_symbol
            return bytes_[offset_(trans) + 2];
        }
        bool is_used_DAC_(size_t trans) const {
            return dac_flags_.get(trans) == 1;
        }
        // TODO: DAC
        size_t offset_rank_(size_t rank) const {
            return rank * dac_flow_size_();
        }
        
        size_t rank_(size_t trans) const {
            return dac_flags_.rank(trans);
        }
    };
    
}

#endif //ARRAY_FSA_ArrayDACFSA_HPP

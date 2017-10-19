//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_ARRAYFSA_HPP
#define ARRAY_FSA_ARRAYFSA_HPP

#include "basic.hpp"

namespace array_fsa {
    
    class ArrayFSA {
        friend class ArrayFSABuilder;
    public:
        ArrayFSA() = default;
        ~ArrayFSA() = default;
        
        ArrayFSA(ArrayFSA&& rhs) noexcept : ArrayFSA() {
            this->swap(rhs);
        }
        ArrayFSA& operator=(ArrayFSA&& rhs) noexcept {
            this->swap(rhs);
            return *this;
        }
        
        static std::string name() {
            return "ArrayFSA";
        }
        
        size_t get_root_state() const {
            return get_next_(0);
        }
        size_t get_trans(size_t state, uint8_t symbol) const {
            const auto trans = state ^ symbol;
            return get_check_(trans) != symbol ? 0 : trans;
        }
        size_t get_target_state(size_t trans) const {
            return get_next_(trans);
        }
        bool is_final_trans(size_t trans) const {
            return (bytes_[trans * element_size_] & 1) == 1;
        }
        
        size_t get_num_trans() const {
            return num_trans_;
        }
        size_t get_num_elements() const {
            return bytes_.size() / element_size_;
        }
        
        void calc_next_size(size_t num_elems) {
            next_size_ = 0;
            while (num_elems >> (8 * ++next_size_ - 1));
        }
        
        void write(std::ostream& os) const {
            write_vec(bytes_, os);
            write_val(next_size_, os);
            write_val(num_trans_, os);
        }
        void read(std::istream& is) {
            bytes_ = read_vec<uint8_t>(is);
            next_size_ = read_val<size_t>(is);
            element_size_ = next_size_ + 1;
            num_trans_ = read_val<size_t>(is);
        }
        
        size_t size_in_bytes() const {
            return size_vec(bytes_) + sizeof(next_size_) + sizeof(num_trans_);
        }
        
        void show_stat(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << get_num_trans() << endl;
            os << "#elems: " << get_num_elements() << endl;
            os << "size:   " << size_in_bytes() << endl;
        }
        
        void swap(ArrayFSA& rhs) {
            bytes_.swap(rhs.bytes_);
            std::swap(next_size_, rhs.next_size_);
            std::swap(element_size_, rhs.element_size_);
            std::swap(num_trans_, rhs.num_trans_);
        }
        
        ArrayFSA(const ArrayFSA&) = delete;
        ArrayFSA& operator=(const ArrayFSA&) = delete;
        
    private:
        std::vector<uint8_t> bytes_;
        size_t next_size_ = 0;
        size_t element_size_ = 0;
        size_t num_trans_ = 0;
        
        size_t get_next_(size_t trans) const { // == get_target_state
            size_t next = 0;
            std::memcpy(&next, &bytes_[trans * element_size_], next_size_);
            return next >> 1;
        }
        uint8_t get_check_(size_t trans) const { // == get_trans_symbol
            return bytes_[trans * element_size_ + next_size_];
        }
    };
    
}

#endif //ARRAY_FSA_ARRAYFSA_HPP

//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_ARRAYFSA_HPP
#define ARRAY_FSA_ARRAYFSA_HPP

#include "ArrayFSABuilder.hpp"

#include "basic.hpp"

namespace array_fsa {
    
    class ArrayFSA {
        friend class ArrayFSABuilder;
    public:
        using Builder = ArrayFSABuilder;
        
        ArrayFSA() = default;
        virtual ~ArrayFSA() = default;
        
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
        
        size_t offset_(size_t trans) const {
            return trans * element_size_;
        }
        
        bool isMember(const std::string& str) const {
            size_t state = get_root_state(), arc = 0;
            
            for (char c : str) {
                arc = get_trans(state, static_cast<uint8_t>(c));
                if (arc == 0) return false;
                state = get_target_state(arc);
            }
            
            return is_final_trans(arc);
        }
        
        size_t get_root_state() const {
            return get_next_(0);
        }
        size_t get_trans(size_t state, uint8_t symbol) const {
            const auto trans = state ^ symbol;
            auto check = get_check_(trans);
            auto is_match = check == symbol;
            return is_match ? trans : 0;
        }
        size_t get_target_state(size_t trans) const {
            return trans ^ get_next_(trans);
        }
        virtual bool is_final_trans(size_t trans) const {
            return (bytes_[offset_(trans)] & 1) != 0;
        }
        
        size_t get_num_trans() const {
            return num_trans_;
        }
        size_t get_num_elements() const {
            return bytes_.size() / element_size_;
        }
        
        virtual void calc_next_size(size_t num_elems) {
            next_size_ = 0;
            while ((num_elems - 1) >> (8 * ++next_size_ - 1));
        }
        
        virtual void read(std::istream& is) {
            bytes_ = read_vec<uint8_t>(is);
            next_size_ = read_val<size_t>(is);
            element_size_ = next_size_ + 1;
            num_trans_ = read_val<size_t>(is);
        }
        virtual void write(std::ostream& os) const {
            write_vec(bytes_, os);
            write_val(next_size_, os);
            write_val(num_trans_, os);
        }
        
        virtual size_t size_in_bytes() const {
            return size_vec(bytes_) + sizeof(next_size_) + sizeof(num_trans_);
        }
        
        virtual void showStatus(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << get_num_trans() << endl;
            os << "#elems: " << get_num_elements() << endl;
            os << "size:   " << size_in_bytes() << endl;
            os << "size bytes_:   " << size_vec(bytes_) << endl;
        }
        
        void swap(ArrayFSA& rhs) {
            bytes_.swap(rhs.bytes_);
            std::swap(next_size_, rhs.next_size_);
            std::swap(element_size_, rhs.element_size_);
            std::swap(num_trans_, rhs.num_trans_);
        }
        
        ArrayFSA(const ArrayFSA&) = delete;
        ArrayFSA& operator=(const ArrayFSA&) = delete;
        
    protected:
        std::vector<uint8_t> bytes_;
        size_t next_size_ = 0;
        size_t element_size_ = 0;
        size_t num_trans_ = 0;
        
        virtual size_t get_next_(size_t trans) const { // == get_target_state
            size_t next = 0;
            std::memcpy(&next, &bytes_[offset_(trans)], next_size_);
            return next >> 1;
        }
        
        virtual uint8_t get_check_(size_t trans) const { // == get_trans_symbol
            return bytes_[offset_(trans) + next_size_];
        }
        
    };
    
}

#endif //ARRAY_FSA_ARRAYFSA_HPP

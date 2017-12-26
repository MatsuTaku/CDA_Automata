//
//  ArrayFSATail.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#ifndef ArrayFSATail_hpp
#define ArrayFSATail_hpp

#include "ArrayFSA.hpp"

#include "Rank.hpp"
#include "DacUnit.hpp"

namespace array_fsa {
    
    class ArrayFSATailBuilder;
    
    class ArrayFSATail : public ArrayFSA {
        friend class ArrayFSATailBuilder;
        
    public:
        ArrayFSATail() = default;
        virtual ~ArrayFSATail() = default;
        
        ArrayFSATail(ArrayFSATail &&rhs) noexcept : ArrayFSATail() {
            this->swap(rhs);
        }
        ArrayFSATail &operator=(ArrayFSATail &rhs) noexcept {
            this->swap(rhs);
            return *this;
        }
        
        using Builder = ArrayFSATailBuilder;
        
        static std::string name() {
            return "ArrayTailFSA";
        }
        
        bool isMember(const std::string &str) const;
        
        // MARK: - getter
        
        size_t get_label_index(size_t trans) const {
            size_t labelIndex = get_check_(trans);
            // TODO: DAC
            /*
            if (dac_check_unit_.get(trans)) {
                labelIndex |= dac_check_unit_.getByte(trans) << 8;
            }
             */
            labelIndex |= dac_check_unit_.getByte(trans) << 8;
            return labelIndex;
        }
        
        bool is_final_trans(size_t trans) const override {
            return is_final_bits_.get(trans);
        }
        
        bool has_label(size_t trans) const {
//            return has_label_bits_.get(trans);
            return dac_check_unit_.getBit(trans);
        }
        
        bool is_label_finish(size_t index) const {
            return label_bytes_[index + 1] == '\0';
        }
        
        // MARK: - setter
        
        virtual void set_next(size_t trans, size_t next) {
            std::memcpy(&bytes_[offset_(trans)], &next, next_size_);
        }
        
        virtual void set_check(size_t trans, uint8_t check) {
            bytes_[offset_(trans) + next_size_] = check;
        }
        
        void set_label_index(size_t trans, size_t labelIndex) {
            if (!has_label(trans)) {
                set_has_label(trans, true);
            }
            size_t mask = 0xff;
            auto firstUnit = labelIndex & mask;
            set_check(trans, firstUnit);
            /*
            auto indexSize = 0;
            while (labelIndex >> (8 * ++indexSize));
            if (indexSize > 1) {
                dac_check_unit_.setBit(trans, true);
                auto byteLink = (labelIndex >> 8);
                dac_check_unit_.setOverByte(byteLink);
            }
             */
            auto byteLink = (labelIndex >> 8);
            dac_check_unit_.setByte(byteLink);
        }
        
        void set_is_final(size_t trans, bool isFinal) {
            is_final_bits_.set(trans, isFinal);
        }
        
        void set_has_label(size_t trans, bool hasLabel) {
//            has_label_bits_.set(trans, hasLabel);
            dac_check_unit_.setBit(trans, hasLabel);
        }
        
        // MARK: - function
        
        void calc_next_size(size_t num_elems) override {
            next_size_ = 0;
            while ((num_elems - 1) >> (8 * ++next_size_));
        }
        
        void calc_check_size(size_t labelSize) {
            check_size_ = 0;
            while ((labelSize - 1) >> (8 * ++check_size_));
            check_size_--;
            dac_check_unit_.setUnitSize(check_size_);
        }
        
        virtual void buildBits() {
            dac_check_unit_.build();
        }
        
        void read(std::istream &is) override {
            ArrayFSA::read(is);
            element_size_ = next_size_ + 1;
            is_final_bits_.read(is);
            has_label_bits_.read(is);
            label_bytes_ = read_vec<uint8_t>(is);
            check_size_ = read_val<size_t>(is);
            dac_check_unit_.read(is);
        }
        
        void write(std::ostream &os) const override {
            ArrayFSA::write(os);
            is_final_bits_.write(os);
            has_label_bits_.write(os);
            write_vec(label_bytes_, os);
            write_val(check_size_, os);
            dac_check_unit_.write(os);
        }
        
        size_t size_in_bytes() const override {
            auto size = ArrayFSA::size_in_bytes();
            size += is_final_bits_.size_in_bytes();
            size += has_label_bits_.size_in_bytes();
            size += size_vec(label_bytes_);
            size += dac_check_unit_.size_in_bytes();
            size += sizeof(check_size_);
            return size;
        }
        
        void show_stat(std::ostream& os) const override {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << get_num_trans() << endl;
            os << "#elems: " << get_num_elements() << endl;
            os << "size:   " << size_in_bytes() << endl;
            os << "size bytes:   " << size_vec(bytes_) << endl;
            os << "size dac_check_bytes:   " << dac_check_unit_.size_in_bytes() << endl;
            os << "size label:   " << size_vec(label_bytes_) << endl;
        }
        
        void swap(ArrayFSATail &rhs) {
            ArrayFSA::swap(rhs);
            label_bytes_.swap(rhs.label_bytes_);
        }
        
    protected:
        Rank is_final_bits_;
        Rank has_label_bits_;
        std::vector<uint8_t> label_bytes_;
        DacUnit dac_check_unit_;
        size_t check_size_ = 0;
        DacUnit label_access_unit_;
        
        size_t get_next_(size_t trans) const override {
            size_t next = 0;
            std::memcpy(&next, &bytes_[offset_(trans)], next_size_);
            return next;
        }
        
        uint8_t get_check_(size_t trans) const override {
            return bytes_[offset_(trans) + next_size_];
        }
        
    };
    
    inline bool ArrayFSATail::isMember(const std::string &str) const {
        auto begin = 0;
        auto node = 0;
        while (begin != str.size()) {
            uint8_t symbol = str[begin];
            node = get_target_state(node) ^ symbol;
            if (!has_label(node)) {
                auto check = get_check_(node);
                if (symbol != check) {
                    return false;
                }
                begin++;
            } else {
                auto labelIndex = get_label_index(node);
                std::string label;
                auto length = 0;
                auto c = label_bytes_[labelIndex + length];
                while (c != '\0') {
                    label += c;
                    c = label_bytes_[labelIndex + ++length];
                }
                auto symbols = str.substr(begin, length);
                if (symbols != label) {
                    return false;
                }
                begin += length;
            }
        }
        return is_final_trans(node);
    }
    
}


#endif /* ArrayFSATail_hpp */

//
//  ArrayFSATail.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#ifndef ArrayFSATail_hpp
#define ArrayFSATail_hpp

#include "Rank.hpp"
#include "DacUnit.hpp"

namespace array_fsa {
    
    struct TransPointer {
        size_t arc = 0;
        bool onTheLabel = false;
        size_t labelState = 0;
    };
    
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
        
        bool translatePointer(TransPointer &pointer, uint8_t symbol) const;
        
        // MARK: - getter
        
        size_t get_label_number(size_t trans) const {
            size_t labelIndex = get_check_(trans);
            // TODO: DAC
            for (size_t size = 1, index = trans; size < check_size_ + 1; size++) {
                auto &unit = dac_check_units_[size - 1];
                if (!unit.get(index)) {
                    break;
                }
                labelIndex |= unit.getByte(index) << (8 * size);
                index = unit.rank(index) - 1;
            }
            return labelIndex;
        }
        
        size_t get_label_index(size_t trans) const {
            auto number = get_label_number(trans);
            auto index = label_start_flags_.select(number);
            return index;
        }
        
        bool is_final_trans(size_t trans) const override {
            return is_final_bits_.get(trans);
        }
        
        bool has_label(size_t trans) const {
            return has_label_bits_.get(trans);
        }
        
        bool is_label_finish(size_t index) const {
            return label_finish_flags_.get(index);
        }
        
        // MARK: - setter
        
        virtual void set_next(size_t trans, size_t next) {
            std::memcpy(&bytes_[offset_(trans)], &next, next_size_);
        }
        
        virtual void set_check(size_t trans, uint8_t check) {
            bytes_[offset_(trans) + next_size_] = check;
        }
        
        void set_label_number(size_t trans, size_t labelIndex) {
            size_t mask = 0xff;
            auto firstUnit = labelIndex & mask;
            set_check(trans, firstUnit);
            auto indexSize = 0;
            while (labelIndex >> (8 * ++indexSize));
            for (size_t size = 1, index = trans; size < indexSize; size++) {
                auto &unit = dac_check_units_[size - 1];
                unit.setBit(index, true);
                auto byte = (labelIndex >> (8 * size)) & mask;
                unit.setByte(byte);
                index = unit.numBytes() - 1;
            }
            
        }
        
        void set_is_final(size_t trans, bool isFinal) {
            is_final_bits_.set(trans, isFinal);
        }
        
        void set_has_label(size_t trans, bool hasLabel) {
            has_label_bits_.set(trans, hasLabel);
        }
        
        void set_is_label_start(size_t index, bool isStart) {
            label_start_flags_.set(index, isStart);
        }
        
        void set_is_label_finish(size_t index, bool isFinish) {
            label_finish_flags_.set(index, isFinish);
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
            dac_check_units_.resize(check_size_);
        }
        
        virtual void buildBits() {
            label_start_flags_.build(true, true);
            for (auto &u : dac_check_units_) {
                u.build();
            }
        }
        
        void read(std::istream &is) override {
            ArrayFSA::read(is);
            element_size_ = next_size_ + 1;
            is_final_bits_.read(is);
            has_label_bits_.read(is);
            label_bytes_ = read_vec<uint8_t>(is);
            label_start_flags_.read(is);
            label_finish_flags_.read(is);
            check_size_ = read_val<size_t>(is);
            for (auto i = 0; i < check_size_; i++) {
                DacUnit unit;
                unit.read(is);
                dac_check_units_.push_back(unit);
            }
        }
        
        void write(std::ostream &os) const override {
            ArrayFSA::write(os);
            is_final_bits_.write(os);
            has_label_bits_.write(os);
            write_vec(label_bytes_, os);
            label_start_flags_.write(os);
            label_finish_flags_.write(os);
            write_val(check_size_, os);
            for (auto &u : dac_check_units_) {
                u.write(os);
            }
        }
        
        size_t size_in_bytes() const override {
            auto size = 0;
            size += ArrayFSA::size_in_bytes();
            size += is_final_bits_.size_in_bytes();
            size += has_label_bits_.size_in_bytes();
            size += size_vec(label_bytes_);
            size += label_start_flags_.size_in_bytes();
            size += label_finish_flags_.size_in_bytes();
            for (auto &unit : dac_check_units_) {
                size += unit.size_in_bytes();
            }
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
            auto dacSize = 0;
            for (auto &unit : dac_check_units_) {
                dacSize += unit.size_in_bytes();
            }
            os << "size dac_check_bytes:   " << dacSize << endl;
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
        Rank label_start_flags_;
        Rank label_finish_flags_;
        std::vector<DacUnit> dac_check_units_;
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
    
    inline bool ArrayFSATail::translatePointer(TransPointer &pointer, uint8_t symbol) const {
        uint8_t check;
        auto isLabelFinish = false;
        if (!pointer.onTheLabel) {
            pointer.arc = get_target_state(pointer.arc) ^ symbol;
            if (has_label(pointer.arc)) {
                pointer.onTheLabel = true;
                pointer.labelState = get_label_index(pointer.arc);
                check = label_bytes_[pointer.labelState];
            } else {
                check = get_check_(pointer.arc);
            }
        } else {
            isLabelFinish = is_label_finish(pointer.labelState);
            if (!isLabelFinish) {
                pointer.labelState++;
                check = label_bytes_[pointer.labelState];
            } else {
                pointer.arc = get_target_state(pointer.arc) ^ symbol;
                if (has_label(pointer.arc)) {
                    pointer.labelState = get_label_index(pointer.arc);
                    check = label_bytes_[pointer.labelState];
                } else {
                    pointer.onTheLabel = false;
                    check = get_check_(pointer.arc);
                }
            }
        }
        auto isMatch = check == symbol;
        if (!isMatch) {
            auto number = get_label_number(pointer.arc);
            auto index = get_label_index(pointer.arc);
            std::cout << number << ":\t" << index << std::endl;
            for (auto i = -10; i < 0; i++) {
                std::cout << " ";
            }
            std::cout << symbol << std::endl;
            for (auto i = -10; i < 10; i++) {
                std::cout << label_bytes_[i + index];
            }
            std::cout << std::endl;
        }
        return isMatch;
    }
    
}


#endif /* ArrayFSATail_hpp */

//
//  ArrayFSATail.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#ifndef ArrayFSATail_hpp
#define ArrayFSATail_hpp

#include "Rank.hpp"

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
        
        size_t get_label_index(size_t index) const {
            size_t labelIndex = get_check_(index);
            // TODO: DAC
            if (dac_check_bits_.get(index)) {
                size_t nextFlow = get_dac_check_flow(index);
                labelIndex |= nextFlow << 8;
            }
            return labelIndex;
        }
        
        size_t offsetDacCheck(size_t index) const {
            return (dac_check_bits_.rank(index) - 1) * check_size_;
        }
        
        size_t get_dac_check_flow(size_t trans) const {
            size_t flow = 0;
            std::memcpy(&flow, &dac_check_bytes_[offsetDacCheck(trans)], check_size_);
            return flow;
        }
        
        std::string get_label(size_t index) const {
            auto labelPos = get_label_index(index);
            std::string label;
            for (auto i = 0; !is_label_finish(labelPos + i); i++) {
                label[i] = label_bytes_[labelPos + i];
            }
            return label;
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
        
        void set_next(size_t trans, size_t next) {
            std::memcpy(&bytes_[offset_(trans)], &next, next_size_);
        }
        
        void set_check(size_t trans, uint8_t check) {
            bytes_[offset_(trans) + next_size_] = check;
        }
        
        void set_used_dac_check(size_t trans, bool useDac) {
            dac_check_bits_.set(trans, useDac);
        }
        
        void set_label_index(size_t trans, size_t labelIndex) {
            size_t first_unit_mask = (size_t(1) << 8) - 1;
            auto firstUnitCheck = labelIndex & first_unit_mask;
            set_check(trans, firstUnitCheck);
            auto flow = labelIndex >> 8;
            if (flow > 0) {
                set_used_dac_check(trans, true);
                for (auto i = 0; i < check_size_; i++) {
                    auto byte = flow >> (i * 8) & 0xff;
                    dac_check_bytes_.push_back(byte);
                }
            }
        }
        
        void set_is_final(size_t trans, bool isFinal) {
            is_final_bits_.set(trans, isFinal);
        }
        
        void set_has_label(size_t trans, bool hasLabel) {
            has_label_bits_.set(trans, hasLabel);
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
        }
        
        void read(std::istream &is) override {
            ArrayFSA::read(is);
            element_size_ = next_size_ + 1;
            is_final_bits_.read(is);
            has_label_bits_.read(is);
            label_bytes_ = read_vec<uint8_t>(is);
            label_finish_flags_.read(is);
            dac_check_bytes_ = read_vec<uint8_t>(is);
            dac_check_bits_.read(is);
            check_size_ = read_val<size_t>(is);
        }
        
        void write(std::ostream &os) const override {
            ArrayFSA::write(os);
            is_final_bits_.write(os);
            has_label_bits_.write(os);
            write_vec(label_bytes_, os);
            label_finish_flags_.write(os);
            write_vec(dac_check_bytes_, os);
            dac_check_bits_.write(os);
            write_val(check_size_, os);
        }
        
        size_t size_in_bytes() const override {
            return ArrayFSA::size_in_bytes() + is_final_bits_.size_in_bytes() + has_label_bits_.size_in_bytes() + size_vec(label_bytes_) + label_finish_flags_.size_in_bytes() + size_vec(dac_check_bytes_) + dac_check_bits_.size_in_bytes() + sizeof(check_size_);
        }
        
        void show_stat(std::ostream& os) const override {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << get_num_trans() << endl;
            os << "#elems: " << get_num_elements() << endl;
            os << "size:   " << size_in_bytes() << endl;
            os << "size bytes:   " << size_vec(bytes_) << endl;
            os << "size dac_check_bytes:   " << size_vec(dac_check_bytes_) << endl;
            os << "size label:   " << size_vec(label_bytes_) << endl;
        }
        
        void swap(ArrayFSATail &rhs) {
            ArrayFSA::swap(rhs);
            label_bytes_.swap(rhs.label_bytes_);
            dac_check_bytes_.swap(rhs.dac_check_bytes_);
        }
        
    private:
        Rank is_final_bits_;
        Rank has_label_bits_;
        std::vector<uint8_t> label_bytes_;
        Rank label_finish_flags_;
        std::vector<uint8_t> dac_check_bytes_;
        Rank dac_check_bits_;
        size_t check_size_ = 0;
        
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
        return isMatch;
    }
    
}


#endif /* ArrayFSATail_hpp */

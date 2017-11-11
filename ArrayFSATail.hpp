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
            size_t labelIndex = 0;
            std::memcpy(&labelIndex, &bytes_[offset_(index) + 1 + next_size_], 4);
            return labelIndex;
        }
        
        std::string get_label(size_t index) const {
            auto labelPos = get_label_index(index);
            std::string label;
            for (auto i = 0; !is_label_finish(labelPos + i); i++) {
                label[i] = label_bytes_[labelPos + i];
            }
            return label;
        }
        
        bool is_final(size_t trans) const {
            return (bytes_[offset_(trans)] & 1) != 0;
        }
        
        bool has_label(size_t trans) const {
            return (bytes_[offset_(trans)] & 2) != 0;
        }
        
        bool is_label_finish(size_t index) const {
            return label_finish_flags_.get(index);
        }
        
        // MARK: - setter
        
        void set_next(size_t trans, size_t next) {
            std::memcpy(&bytes_[offset_(trans) + 1], &next, next_size_);
        }
        
        void set_check(size_t trans, uint8_t check) {
            bytes_[offset_(trans) + 1 + next_size_] = check;
        }
        
        void set_label_index(size_t trans, size_t labelIndex) {
            std::memcpy(&bytes_[offset_(trans) + 1 + next_size_], &labelIndex, 4);
        }
        
        void set_is_final(size_t trans, bool isFinal) {
            if (isFinal) {
                bytes_[offset_(trans)] |= 1;
            } else {
                bytes_[offset_(trans)] &= ~1;
            }
        }
        
        void set_has_label(size_t trans, bool hasLabel) {
            if (hasLabel) {
                bytes_[offset_(trans)] |= 2;
            } else {
                bytes_[offset_(trans)] &= ~2;
            }
        }
        
        void set_is_label_finish(size_t index, bool isFinish) {
            label_finish_flags_.set(index, isFinish);
        }
        
        // MARK: - function
        
        void read(std::istream &is) override {
            ArrayFSA::read(is);
            element_size_ = next_size_ + 1 + 4;
            label_bytes_ = read_vec<uint8_t>(is);
            label_finish_flags_.read(is);
        }
        
        void write(std::ostream &os) const override {
            ArrayFSA::write(os);
            write_vec(label_bytes_, os);
            label_finish_flags_.write(os);
        }
        
        size_t size_in_bytes() const override {
            return ArrayFSA::size_in_bytes() + size_vec(label_bytes_) + label_finish_flags_.size_in_bytes();
        }
        
        void show_stat(std::ostream& os) const override {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << get_num_trans() << endl;
            os << "#elems: " << get_num_elements() << endl;
            os << "size:   " << size_in_bytes() << endl;
            os << "size bytes:   " << size_vec(bytes_) << endl;
            os << "size label:   " << size_vec(label_bytes_) << endl;
        }
        
        void swap(ArrayFSATail &rhs) {
            ArrayFSA::swap(rhs);
            label_bytes_.swap(rhs.label_bytes_);
        }
        
    private:
        std::vector<uint8_t> label_bytes_;
        Rank label_finish_flags_;
        
        size_t get_next_(size_t trans) const override {
            size_t next = 0;
            std::memcpy(&next, &bytes_[offset_(trans) + 1], next_size_);
            return next;
        }
        
        uint8_t get_check_(size_t trans) const override {
            return bytes_[offset_(trans) + 1 + next_size_];
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

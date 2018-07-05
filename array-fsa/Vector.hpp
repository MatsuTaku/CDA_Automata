//
//  Vector.hpp
//  build
//
//  Created by 松本拓真 on 2018/05/04.
//

#ifndef Vector_hpp
#define Vector_hpp

#include "basic.hpp"
#include "Calc.hpp"

namespace array_fsa {
    
    /*
     * This is Fit to binary size of max-value in source vector integers.
     */
    class Vector {
    public:
        using id_type = uint64_t;
        static constexpr size_t kBitsSizeInElement = 8 * sizeof(id_type); // 64
    public:
        // MARK: - Constructor
        Vector(size_t typeSize = 1) : kBitsSizeOfTypes_(typeSize), kMask_(maskInSize(typeSize)) {}
        Vector(size_t typeSize, size_t size) : kBitsSizeOfTypes_(typeSize), kMask_(maskInSize(typeSize)) {
            resize(size);
        }
        Vector(size_t typeSize, size_t size, size_t value) : kBitsSizeOfTypes_(typeSize), kMask_(maskInSize(typeSize)) {
            assign(size, value);
        }
        
        Vector(std::istream &is) : kBitsSizeOfTypes_(read_val<size_t>(is)), kMask_(maskInSize(kBitsSizeOfTypes_)) {
            size_ = read_val<size_t>(is);
            vector_ = read_vec<uint64_t>(is);
        }
        
        template<typename T>
        Vector(const std::vector<T> &vector) : kBitsSizeOfTypes_(typeSizeOfVector(vector)), kMask_(maskInSize(kBitsSizeOfTypes_)) {
            resize(vector.size());
            for (auto i = 0; i < vector.size(); i++) {
                set(i, vector[i]);
            }
        }
        // Used at constructor
        template<typename T>
        size_t typeSizeOfVector(const std::vector<T> &vector) const {
            auto maxV = *std::max_element(vector.begin(), vector.end());
            return Calc::sizeFitInBits(maxV);
        }
        
        size_t maskInSize(size_t size) const {
            return (1U << size) - 1;
        }
        
        ~Vector() = default;
        
        Vector(const Vector &rhs) = delete;
        Vector& operator=(const Vector &rhs) = delete;
        
        Vector(Vector &&rhs) : kBitsSizeOfTypes_(rhs.kBitsSizeOfTypes_), kMask_(rhs.kMask_) {
            size_ = rhs.size_;
            vector_ = std::move(rhs.vector_);
        }
        Vector& operator=(Vector &&rhs) noexcept {
            // const members
            size_t *ptr = const_cast<size_t*>(&this->kBitsSizeOfTypes_);
            *ptr = rhs.kBitsSizeOfTypes_;
            ptr = const_cast<size_t*>(&this->kMask_);
            *ptr = rhs.kMask_;
            // var members
            size_ = std::move(rhs.size_);
            vector_ = std::move(rhs.vector_);
            
            return *this;
        }
        
        constexpr size_t operator[](size_t index) const {
            auto abs = abs_(index);
            auto rel = rel_(index);
            if (kBitsSizeInElement >= rel + kBitsSizeOfTypes_)
                return (vector_[abs] >> rel) & kMask_;
            else
                return ((vector_[abs] >> rel) | (vector_[abs + 1] << (kBitsSizeInElement - rel))) & kMask_;
        }
        
        void set(size_t index, size_t value) {
            auto abs = abs_(index);
            auto rel = rel_(index);
            vector_[abs] &= ~(kMask_ << rel);
            vector_[abs] |= value << rel;
            if (kBitsSizeOfTypes_ + rel > kBitsSizeInElement) {
                vector_[abs + 1] &= ~(kMask_ >> (kBitsSizeInElement - rel));
                vector_[abs + 1] |= value >> (kBitsSizeInElement - rel);
            }
        }
        
        size_t size() const {
            return size_;
        }
        
        void push_back(size_t value) {
            resize(size_ + 1);
            set(size_ - 1, value);
        }
        
        void resize(size_t size) {
            if (size <= size_) return;
            auto abs = abs_(size - 1);
            if (vector_.size() == 0 || abs > vector_.size() - 1)
                vector_.resize(abs + 1);
            else if (abs == vector_.size() - 1 && rel_(size - 1) + kBitsSizeOfTypes_ > kBitsSizeInElement)
                vector_.resize(abs + 2);
            size_ = size;
        }
        
        void assign(size_t size, size_t value) {
            vector_.resize(size);
            for (auto i = 0; i < size; i++)
                set(i, value);
        }
        
        size_t sizeInBytes() const {
            auto size = sizeof(kBitsSizeOfTypes_) + sizeof(size_);
            size += size_vec(vector_);
            return size;
        }
        
        void write(std::ostream &os) const {
            write_val(kBitsSizeOfTypes_, os);
            write_val(size_, os);
            write_vec(vector_, os);
        }
        
    private:
        const size_t kBitsSizeOfTypes_;
        const size_t kMask_;
        size_t size_ = 0;
        std::vector<id_type> vector_;
        
        constexpr size_t abs_(size_t index) const {
            return index * kBitsSizeOfTypes_ / kBitsSizeInElement;
        }
        
        constexpr size_t rel_(size_t index) const {
            return index * kBitsSizeOfTypes_ % kBitsSizeInElement;
        }
        
    };
    
}


#endif /* Vector_hpp */

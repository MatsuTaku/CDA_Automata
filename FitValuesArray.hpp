//
//  FitValuesArray.hpp
//  bench
//
//  Created by 松本拓真 on 2018/01/05.
//

#ifndef FitValuesArray_hpp
#define FitValuesArray_hpp

#include "ByteData.hpp"

namespace array_fsa {
    
    class FitValuesArray : ByteData {
    public:
        FitValuesArray() = default;
        ~FitValuesArray() = default;
        
        FitValuesArray(FitValuesArray &&rhs) noexcept : FitValuesArray() {
            this->swap(rhs);
        }
        FitValuesArray& operator=(FitValuesArray &&rhs) noexcept {
            this->swap(rhs);
            return *this;
        }
        
        FitValuesArray(const FitValuesArray&) = delete;
        FitValuesArray& operator=(const FitValuesArray&) = delete;
        
        size_t offset(size_t index) const {
            return index * element_size_;
        }
        
        size_t get_element_size_() const {
            return element_size_;
        }
        
        size_t numElements() const {
            return bytes_.size() / element_size_;
        }
        
        template <typename T>
        T getValue(size_t index, size_t num) const {
            T value = 0;
            auto pos = offset(index) + value_positions_[num];
            std::memcpy(&value, &bytes_[pos], value_sizes_[num]);
            return value;
        }
        
        template <typename T>
        void setValue(size_t index, size_t num, T value) {
            auto pos = offset(index) + value_positions_[num];
            std::memcpy(&bytes_[pos], &value, value_sizes_[num]);
        }
        
        void insertValueSize(size_t index, size_t size) {
            element_size_ += size;
            value_sizes_.insert(value_sizes_.begin() + index, size);
            
            auto pos = value_positions_.size() > 0 ? value_positions_[index - 1] + value_sizes_[index - 1] : 0;
            value_positions_.insert(value_positions_.begin() + index, pos);
            if (index == value_positions_.size() - 1) return;
            for (auto i = index + 1; i < value_positions_.size(); i++) {
                value_positions_[i] += size;
            }
        }
        
        void resize(size_t indexSize) {
            bytes_.resize(offset(indexSize));
        }
        
        size_t sizeInBytes() const override {
            auto size = size_vec(bytes_);
            size += size_vec(value_sizes_);
            return size;
        }
        
        size_t sizeBytes() const {
            return size_vec(bytes_);
        }
        
        void write(std::ostream &os) const override {
            write_vec(bytes_, os);
            write_vec(value_sizes_, os);
        }
        
        void read(std::istream &is) override {
            bytes_ = read_vec<uint8_t>(is);
            
            auto sizes = read_vec<uint8_t>(is);
            for (auto i = 0; i < sizes.size(); i++) {
                insertValueSize(i, sizes[i]);
            }
        }
        
        void swap(FitValuesArray &rhs) {
            bytes_.swap(rhs.bytes_);
            value_sizes_.swap(rhs.value_sizes_);
            std::swap(element_size_, rhs.element_size_);
            value_positions_.swap(rhs.value_positions_);
        }
        
    private:
        std::vector<uint8_t> bytes_ = {};
        std::vector<uint8_t> value_sizes_ = {};
        uint8_t element_size_ = 0;
        std::vector<size_t> value_positions_ = {};
        
    };
    
}

#endif /* FitValuesArray_hpp */

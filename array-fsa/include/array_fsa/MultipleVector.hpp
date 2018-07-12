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
    
    class MultipleVector : ByteData {
    public:
        // MARK: copy guard
        MultipleVector() = default;
        ~MultipleVector() = default;
        
        MultipleVector(const MultipleVector&) = delete;
        MultipleVector& operator=(const MultipleVector&) = delete;
        
        MultipleVector(MultipleVector &&rhs) noexcept = default;
        MultipleVector& operator=(MultipleVector &&rhs) noexcept = default;
        
    public:
        using IdType = uint8_t;
        
        void setValueSize(size_t index, size_t size);
        
        template <typename T>
        void setValueSizes(std::vector<T>& sizes) {
            value_sizes_ = {};
            value_positions_ = {};
            element_size_ = 0;
            for (auto i = 0; i < sizes.size(); i++)
                setValueSize(i, sizes[i]);
        }
        
        size_t offset(size_t index) const {
            return index * element_size_;
        }
        
        size_t elementSize() const {
            return element_size_;
        }
        
        size_t size() const {
            return bytes_.size() / element_size_;
        }
        
        template <int N, typename T>
        T get(size_t index) const;
        
        template <int N, typename T>
        void set(size_t index, T value);
        
        void resize(size_t indexSize) {
            bytes_.resize(offset(indexSize));
        }
        
        // MARK: - ByteData method
        
        size_t sizeInBytes() const override {
            auto size = size_vec(bytes_);
            size += size_vec(value_sizes_);
            return size;
        }
        
        void write(std::ostream &os) const override {
            write_vec(bytes_, os);
            write_vec(value_sizes_, os);
        }
        
        void read(std::istream &is) override {
            bytes_ = read_vec<IdType>(is);
            
            auto sizes = read_vec<uint8_t>(is);
            for (auto i = 0; i < sizes.size(); i++)
                setValueSize(i, sizes[i]);
        }
        
    private:
        std::vector<IdType> bytes_ = {};
        
        uint8_t element_size_ = 0;
        std::vector<uint8_t> value_sizes_ = {};
        std::vector<size_t> value_positions_ = {};
        
    };
    
    // MARK: - inline function
    
    inline void MultipleVector::setValueSize(size_t index, size_t size) {
        element_size_ += size;
        value_sizes_.insert(value_sizes_.begin() + index, size);
        
        auto pos = value_positions_.size() > 0 ? value_positions_[index - 1] + value_sizes_[index - 1] : 0;
        value_positions_.insert(value_positions_.begin() + index, pos);
        
        if (index == value_positions_.size() - 1)
            return;
        for (auto i = index + 1; i < value_positions_.size(); i++)
            value_positions_[i] += size;
    }
    
    template <int N, typename T>
    inline T MultipleVector::get(size_t index) const {
        assert(sizeof(T) >= value_sizes_[N]);
        T value = 0;
        auto pos = offset(index) + value_positions_[N];
        for (size_t i = 0, size = value_sizes_[N]; i < size; i++)
            value |= T(bytes_[pos + i]) << (i * 8);
        return value;
    }
    
    template <int N, typename T>
    inline void MultipleVector::set(size_t index, T value) {
        assert(sizeof(T) >= value_sizes_[N]);
        auto pos = offset(index) + value_positions_[N];
        for (auto i = 0; i < value_sizes_[N]; i++)
            bytes_[pos + i] = static_cast<IdType>(value >> (8 * i));
    }
    
}

#endif /* FitValuesArray_hpp */

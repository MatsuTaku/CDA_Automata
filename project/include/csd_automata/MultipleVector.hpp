//
//  FitValuesArray.hpp
//  bench
//
//  Created by 松本拓真 on 2018/01/05.
//

#ifndef FitValuesArray_hpp
#define FitValuesArray_hpp

#include "basic.hpp"
#include "IOInterface.hpp"

namespace csd_automata {
    
    class MultipleVector : IOInterface {
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
        
        template <typename CONTAINER>
        void setValueSizes(CONTAINER& sizes) {
            value_sizes_ = {};
            for (auto i = 0; i < sizes.size(); i++)
                value_sizes_.push_back(sizes[i]);
            
            value_positions_ = {};
            for (auto i = 0, pos = 0; i < sizes.size(); i++) {
                value_positions_.push_back(pos);
                pos += sizes[i];
            }
            element_size_ = value_positions_.back() + value_sizes_.back();
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
        
        size_t valueSize(size_t offset) const {
            return value_sizes_[offset];
        }
        
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
            setValueSizes(sizes);
        }
        
    private:
        std::vector<IdType> bytes_ = {};
        
        uint8_t element_size_ = 0;
        std::vector<uint8_t> value_sizes_ = {};
        std::vector<size_t> value_positions_ = {};
        
    };
    
    
    // MARK: - inline function
    
    template <int N, typename T>
    inline T MultipleVector::get(size_t index) const {
        T value = 0;
        auto pos = offset(index) + value_positions_[N];
        for (size_t i = 0, size = std::min(value_sizes_[N], uint8_t(sizeof(T))); i < size; i++)
            value |= T(bytes_[pos + i]) << (i * 8);
        return value;
    }
    
    
    template <int N, typename T>
    inline void MultipleVector::set(size_t index, T value) {
        auto pos = offset(index) + value_positions_[N];
        for (size_t i = 0, size = std::min(value_sizes_[N], uint8_t(sizeof(T))); i < size; i++)
            bytes_[pos + i] = static_cast<IdType>(value >> (8 * i));
    }
    
}

#endif /* FitValuesArray_hpp */

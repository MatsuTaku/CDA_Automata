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

#include "sim_ds/calc.hpp"

namespace csd_automata {
    
class MultipleVector : IOInterface {
    using storage_type = uint8_t;
    static constexpr size_t kBitsPerWord = sizeof(storage_type) * 8;
    
    std::vector<size_t> value_sizes_ = {};
    std::vector<size_t> value_positions_ = {};
    std::vector<storage_type> bytes_ = {};
    
public:
    template <typename T>
    void set_value_sizes(std::vector<T>& sizes) {
        value_sizes_ = {};
        for (auto i = 0; i < sizes.size(); i++)
            value_sizes_.push_back(sizes[i]);
        
        value_positions_ = {};
        for (auto i = 0, pos = 0; i < sizes.size(); i++) {
            value_positions_.push_back(pos);
            pos += sizes[i];
        }
    }
    
    size_t element_size() const {
        return value_positions_.back() + value_sizes_.back();
    }
    
    size_t size() const {
        return bytes_.size() / element_size();
    }
    
    size_t value_size(size_t offset) const {
        return value_sizes_[offset];
    }
    
    template <int Id, typename T = id_type>
    void set(size_t index, T value) {
        auto vs = value_size(Id);
        assert(vs == 8 ||
               sim_ds::calc::SizeFitsInUnits(value, vs * kBitsPerWord) == 1);
        
        auto pos = offset_(index) + value_positions_[Id];
        for (size_t i = 0, size = std::min(vs, sizeof(T)); i < size; i++)
            bytes_[pos + i] = static_cast<storage_type>(value >> (i * kBitsPerWord));
    }
    
    template <int Id, typename T = id_type>
    T get(size_t index) const {
        T value = 0;
        auto pos = offset_(index) + value_positions_[Id];
        auto size = std::min(value_size(Id), sizeof(T));
        for (size_t i = 0; i < size; i++)
            value |= T(bytes_[pos + i]) << (i * kBitsPerWord);
        
        return value;
    }
    
    void resize(size_t indexSize) {
        bytes_.resize(offset_(indexSize));
    }
    
    // MARK: IO
    
    size_t size_in_bytes() const override {
        auto size = size_vec(bytes_);
        size += size_vec(value_sizes_);
        return size;
    }
    
    void Read(std::istream& is) override {
        bytes_ = read_vec<storage_type>(is);
        
        value_sizes_ = read_vec<size_t>(is);
        value_positions_ = {};
        for (auto i = 0, pos = 0; i < value_sizes_.size(); i++) {
            value_positions_.push_back(pos);
            pos += value_sizes_[i];
        }
    }
    
    void Write(std::ostream& os) const override {
        write_vec(bytes_, os);
        write_vec(value_sizes_, os);
    }
    
    // MARK: copy guard
    
    MultipleVector() = default;
    ~MultipleVector() = default;
    
    MultipleVector(const MultipleVector&) = delete;
    MultipleVector& operator=(const MultipleVector&) = delete;
    
    MultipleVector(MultipleVector &&rhs) noexcept = default;
    MultipleVector& operator=(MultipleVector &&rhs) noexcept = default;
    
private:
    size_t offset_(size_t index) const {
        return index * element_size();
    }
    
    
};
    
}

#endif /* FitValuesArray_hpp */

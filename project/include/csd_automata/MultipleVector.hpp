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


template <class SerializedSequence>
class BlockReference {
    using Storage = SerializedSequence;
    using storage_type = typename SerializedSequence::storage_type;
    using storage_pointer = typename SerializedSequence::storage_pointer;
    
    static constexpr size_t kBitsPerWord = Storage::kBitsPerWord;
    
    storage_pointer pointer_;
    
    using params_type = typename SerializedSequence::params_type;
    using params_reference = const params_type&;
    params_reference element_sizes_;
    params_reference element_positions_;
    
    friend typename SerializedSequence::Self;
    
public:
    template <int Id>
    id_type get() const {
        return get_<Id, id_type>(element_sizes_[Id]);
    }
    
    template <int Id, typename T>
    T restricted_get() const {
        return get_<Id, T>(std::min(element_sizes_[Id], sizeof(T)));
    }
    
    template <int Id>
    id_type set(id_type value) {
        return set_<Id, id_type>(element_sizes_[Id], value);
    }
    
    template <int Id, typename T>
    T restricted_set(T value) {
        return set_<Id, T>(std::min(element_sizes_[Id], sizeof(T)), value);
    }
    
private:
    explicit BlockReference(storage_pointer pointer, params_reference element_sizes, params_reference element_positions) : pointer_(pointer), element_sizes_(element_sizes), element_positions_(element_positions) {}
    
    template <int Id, typename T>
    T get_(size_t width) const {
        assert(Id < element_sizes_.size());
        assert(width <= element_sizes_[Id]);
        
        id_type value = 0;
        auto relative_pointer = pointer_ + element_positions_[Id];
        for (size_t i = 0; i < width; i++)
            value |= static_cast<id_type>(*(relative_pointer + i)) << (i * kBitsPerWord);
        
        return value;
    }
    
    template <int Id, typename T>
    T set_(size_t width, T value) {
        assert(Id < element_sizes_.size());
        assert(width <= element_sizes_[Id]);
        
        auto relative_pointer = pointer_ + element_positions_[Id];
        for (size_t i = 0; i < width; i++)
            *(relative_pointer + i) = static_cast<storage_type>(value >> (i * kBitsPerWord));
        
        return value;
    }
    
};


template <class SerializedSequence>
class BlockConstReference {
    using Storage = SerializedSequence;
    using storage_type = typename SerializedSequence::storage_type;
    using storage_pointer = typename SerializedSequence::const_storage_pointer;
    static constexpr size_t kBitsPerWord = SerializedSequence::kBitsPerWord;
    
    storage_pointer pointer_;
    
    using params_type = typename SerializedSequence::params_type;
    using params_reference = const params_type&;
    params_reference element_sizes_;
    params_reference element_positions_;
    
    friend typename SerializedSequence::Self;
    
public:
    template <int Id, typename T>
    T restricted_get() const {
        return get_<Id, T>(std::min(element_sizes_[Id], sizeof(T)));
    }
    
    template <int Id>
    id_type get() const {
        return get_<Id, id_type>(element_sizes_[Id]);
    }
    
private:
    explicit BlockConstReference(storage_pointer pointer, params_reference element_sizes, params_reference element_positions) : pointer_(pointer), element_sizes_(element_sizes), element_positions_(element_positions) {}
    
    template <int Id, typename T>
    T get_(size_t width) const {
        assert(Id < element_sizes_.size());
        assert(width <= element_sizes_[Id]);
        
        id_type value = 0;
        auto relative_pointer = pointer_ + element_positions_[Id];
        for (size_t i = 0; i < width; i++)
            value |= static_cast<T>(*(relative_pointer + i)) << (i * kBitsPerWord);
        
        return value;
    }
    
};


class MultipleVector : IOInterface {
    using Self = MultipleVector;
    using storage_type = uint8_t;
    using storage_pointer = storage_type*;
    using const_storage_pointer = const storage_type*;
    
    static constexpr size_t kBitsPerWord = sizeof(storage_type) * 8;
    
    using params_type = std::vector<size_t>;
    using Storage = std::vector<storage_type>;
    
    params_type element_sizes_ = {};
    params_type element_positions_ = {};
    Storage bytes_ = {};
    
    using Reference = BlockReference<MultipleVector>;
    using ConstReference = BlockConstReference<MultipleVector>;
    
    friend class BlockReference<MultipleVector>;
    friend class BlockConstReference<MultipleVector>;
    
public:
    template <typename T>
    void set_value_sizes(std::vector<T>& sizes) {
        element_sizes_ = {};
        for (auto i = 0; i < sizes.size(); i++)
            element_sizes_.push_back(sizes[i]);
        
        element_positions_ = {};
        for (auto i = 0, pos = 0; i < sizes.size(); i++) {
            element_positions_.push_back(pos);
            pos += sizes[i];
        }
    }
    
    size_t block_size() const {
        return element_positions_.back() + element_sizes_.back();
    }
    
    size_t size() const {
        return bytes_.size() / block_size();
    }
    
    size_t element_size(size_t offset) const {
        return element_sizes_[offset];
    }
    
    Reference block(size_t index) {
        return Reference(&bytes_[offset_(index)], element_sizes_, element_positions_);
    }
    
    ConstReference block(size_t index) const {
        return ConstReference(&bytes_[offset_(index)], element_sizes_, element_positions_);
    }
    
    template <int Id>
    id_type set_nested_element(size_t index, id_type value) {
        assert(Id < element_sizes_.size());
        assert(index < size());
        assert(element_sizes_[Id] == 8 || // 8 Byte element has no problem.
               sim_ds::calc::SizeFitsInBytes(value) <= element_sizes_[Id]);
        return set_(offset_(index) + element_positions_[Id], element_sizes_[Id], value);
    }
    
    template <int Id>
    id_type nested_element(size_t index) const {
        assert(Id < element_sizes_.size());
        assert(index < size());
        return get_(offset_(index) + element_positions_[Id], element_sizes_[Id]);
    }
    
    void resize(size_t size) {
        bytes_.resize(offset_(size));
    }
    
    // MARK: IO
    
    size_t size_in_bytes() const override {
        auto size = size_vec(bytes_);
        size += size_vec(element_sizes_);
        return size;
    }
    
    void Read(std::istream& is) override {
        bytes_ = read_vec<storage_type>(is);
        
        element_sizes_ = read_vec<size_t>(is);
        element_positions_ = {};
        for (auto i = 0, pos = 0; i < element_sizes_.size(); i++) {
            element_positions_.push_back(pos);
            pos += element_sizes_[i];
        }
    }
    
    void Write(std::ostream& os) const override {
        write_vec(bytes_, os);
        write_vec(element_sizes_, os);
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
        return index * block_size();
    }
    
    id_type set_(size_t offset, size_t width, id_type value) {
        for (size_t i = 0; i < width; i++)
            bytes_[offset + i] = static_cast<storage_type>(value >> (i * kBitsPerWord));
        
        return value;
    }
    
    id_type get_(size_t offset, size_t width) const {
        id_type value = 0;
        for (size_t i = 0; i < width; i++)
            value |= static_cast<id_type>(bytes_[offset + i]) << (i * kBitsPerWord);
        
        return value;
    }
    
    
};
    
}

#endif /* FitValuesArray_hpp */

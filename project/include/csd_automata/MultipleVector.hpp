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
        return get_<Id>(element_sizes_[Id]);
    }
    
    template <int Id, typename T>
    T get() const {
        return get_<Id, T>(std::min(element_sizes_[Id], sizeof(T)));
    }
    
    template <int Id>
    id_type set(id_type value) {
        return set_<Id>(element_sizes_[Id], value);
    }
    
    template <int Id, typename T>
    T set(T value) {
        return set_<Id, T>(std::min(element_sizes_[Id], sizeof(T)), value);
    }
    
private:
    explicit BlockReference(storage_pointer pointer, params_reference element_sizes, params_reference element_positions) : pointer_(pointer), element_sizes_(element_sizes), element_positions_(element_positions) {}
    
    template <int Id, typename T = id_type>
    T get_(size_t width) const {
        assert(Id < element_sizes_.size());
        id_type value = 0;
        auto relative_pointer = pointer_ + element_positions_[Id];
        for (size_t i = 0; i < width; i++)
            value |= static_cast<id_type>(*(relative_pointer + i)) << (i * kBitsPerWord);
        
        return value;
    }
    
    template <int Id, typename T = id_type>
    T set_(size_t width, T value) {
        assert(Id < element_sizes_.size());
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
    template <int Id>
    id_type get() const {
        return get_<Id>(element_sizes_[Id]);
    }
    
    template <int Id, typename T>
    T get() const {
        return get_<Id, T>(std::min(element_sizes_[Id], sizeof(T)));
    }
    
private:
    explicit BlockConstReference(storage_pointer pointer, params_reference element_sizes, params_reference element_positions) : pointer_(pointer), element_sizes_(element_sizes), element_positions_(element_positions) {}
    
    template <int Id, typename T = id_type>
    T get_(size_t width) const {
        assert(Id < element_sizes_.size());
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
    using storage_pointer = uint8_t*;
    using const_storage_pointer = const uint8_t*;
    
    static constexpr size_t kBitsPerWord = sizeof(storage_type) * 8;
    
    using params_type = std::vector<size_t>;
    using Storage = std::vector<storage_type>;
    
    params_type value_sizes_ = {};
    params_type value_positions_ = {};
    Storage bytes_ = {};
    
    using Reference = BlockReference<MultipleVector>;
    using ConstReference = BlockConstReference<MultipleVector>;
    
    friend class BlockReference<MultipleVector>;
    friend class BlockConstReference<MultipleVector>;
    
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
    
    Reference operator[](size_t index) {
        return Reference(&bytes_[offset_(index)], value_sizes_, value_positions_);
    }
    
    ConstReference operator[](size_t index) const {
        return ConstReference(&bytes_[offset_(index)], value_sizes_, value_positions_);
    }
    
    template <int Id>
    id_type set(size_t index, id_type value) {
        auto vs = value_size(Id);
        assert(vs == 8 ||
               sim_ds::calc::SizeFitsInUnits(value, vs * kBitsPerWord) == 1);
        return set_<Id>(index, vs, value);
    }
    
    template <int Id, typename T>
    T set(size_t index, T value) {
        auto vs = value_size(Id);
        assert(vs == 8 ||
               sim_ds::calc::SizeFitsInUnits(value, vs * kBitsPerWord) == 1);
        return set_<Id, T>(index, std::min(vs, sizeof(T)), value);
    }
    
    template <int Id>
    id_type get(size_t index) const {
        return get_<Id>(index, value_size(Id));
    }
    
    template <int Id, typename T>
    T get(size_t index) const {
        return get_<Id, T>(index, std::min(value_size(Id), sizeof(T)));
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
    
    template <int Id, typename T = id_type>
    T set_(size_t index, size_t width, T value) {
        assert(Id < value_sizes_.size());
        auto pos = offset_(index) + value_positions_[Id];
        for (size_t i = 0; i < width; i++)
            bytes_[pos + i] = static_cast<storage_type>(value >> (i * kBitsPerWord));
        return value;
    }
    
    template <int Id, typename T = id_type>
    T get_(size_t index, size_t width) const {
        assert(Id < value_sizes_.size());
        T value = 0;
        auto pos = offset_(index) + value_positions_[Id];
        for (size_t i = 0; i < width; i++)
            value |= static_cast<T>(bytes_[pos + i]) << (i * kBitsPerWord);
        
        return value;
    }
    
    
};
    
}

#endif /* FitValuesArray_hpp */

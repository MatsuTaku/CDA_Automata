//
//  StringArray.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/14.
//

#ifndef StringArray_hpp
#define StringArray_hpp

#include "IOInterface.hpp"
#include "sim_ds/BitVector.hpp"
#include "sim_ds/SuccinctBitVector.hpp"

namespace csd_automata {


template <bool IsBinaryMode, bool SelectAccess> class SerializedStrings;

class SerializedStringsBuilder {
    using char_type = char;
    using BitVector = sim_ds::BitVector;
    
    static constexpr char_type kEndLabel = '\0';
    
    bool binary_mode_;
    
    std::vector<char_type> bytes_;
    BitVector boundary_flags_;
    BitVector popuration_flags_;
    
public:
    SerializedStringsBuilder() = default;
    
    SerializedStringsBuilder(bool binary_mode) : binary_mode_(binary_mode) {}
    
    SerializedStringsBuilder(const SerializedStringsBuilder&) = delete;
    SerializedStringsBuilder& operator=(const SerializedStringsBuilder&) = delete;
    
    SerializedStringsBuilder(SerializedStringsBuilder&&) = default;
    SerializedStringsBuilder& operator=(SerializedStringsBuilder&&) = default;
    
    // MARK: For build
    
    void AddString(std::string_view str) {
        assert(str.size() > 0);
        for (auto c : str) {
            bytes_.push_back(static_cast<char_type>(c));
        }
        if (binary_mode_) {
            boundary_flags_.resize(bytes_.size());
            boundary_flags_[bytes_.size() - 1] = true;
        } else {
            bytes_.push_back(kEndLabel);
        }
    }
    
    void SetPopuration(size_t index) {
        if (popuration_flags_.size() < index + 1)
            popuration_flags_.resize(index + 1);
        popuration_flags_[index] = true;
    }
    
    template <bool IsBinaryMode, bool SelectAccess>
    void Release(SerializedStrings<IsBinaryMode, SelectAccess>& product) {
        assert(IsBinaryMode == binary_mode_);
        if (IsBinaryMode != binary_mode_) {
            std::cout << "StringArray error type of binary mode!!" << std::endl;
            abort();
        }
        product.bytes_ = move(bytes_);
        if (SelectAccess) {
            product.popuration_flags_ = sim_ds::SuccinctBitVector<>(popuration_flags_);
        }
        if (IsBinaryMode) {
            product.boundary_flags_ = boundary_flags_;
        }
    }
    
    // MARK: Parameter
    
    bool is_binary_mode() const {
        return binary_mode_;
    }
    
    auto operator[](size_t index) const {
        return bytes_[index];
    }
    
    auto size() const {
        return bytes_.size();
    }
    
    bool is_back_at(size_t index) const {
        return binary_mode_ ? boundary_flags_[index] : bytes_[index + 1] == kEndLabel;
    }
    
};


template<bool IsBinaryMode, bool SelectAccess>
class SerializedStrings : IOInterface {
    using Self = SerializedStrings<IsBinaryMode, SelectAccess>;
    using char_type = char;
    using Storage = std::vector<char_type>;
    using BitVector = sim_ds::BitVector;
    using SuccinctBitVector = sim_ds::SuccinctBitVector<>;
    
    static constexpr bool kIsBinaryMode = IsBinaryMode;
    static constexpr bool kSelectAccess = SelectAccess;
    
    static constexpr char_type kEndLabel = '\0';
    
    friend class SerializedStringsBuilder;
    
public:
    explicit SerializedStrings(SerializedStringsBuilder& builder) {
        builder.Release(*this);
    }
    
    explicit SerializedStrings(std::istream& is) {
        LoadFrom(is);
    }
    
    // MARK: Property
    
    char_type operator[](size_t index) const {
        return bytes_[index];
    }
    
    bool is_back_at(size_t index) const {
        if constexpr (kIsBinaryMode) {
            return boundary_flags_[index];
        } else
            return bytes_[index + 1] == kEndLabel;
    }
    
    bool match(size_t* pos, std::string_view str, size_t str_id) const {
        size_t str_index = !kSelectAccess ? str_id : index_select(str_id);
        for (; *pos < str.size(); ++*pos, str_index++) {
            auto str_c = static_cast<char>(str[*pos]);
            auto store_c = bytes_[str_index];
            if (str_c != store_c)
                return false;
            if (is_back_at(str_index))
                return true;
        }
        return false;
    }
    
    std::string string(size_t id) const {
        std::string s;
        size_t index = !kSelectAccess ? id : index_select(id);
        for (char c = bytes_[index]; c != kEndLabel; c = bytes_[++index]) {
            s.push_back(c);
        }
        return s;
    }
    
    std::basic_string_view<char_type> string_view(size_t id) const {
        size_t index = !kSelectAccess ? id : index_select(id);
        size_t i = index;
        if constexpr (kIsBinaryMode) {
            while (!boundary_flags_[i++]);
        } else {
            while (bytes_[++i] != kEndLabel);
        }
        return std::basic_string_view<char_type>(&bytes_[index], i - index);
    }
    
    size_t size() const {
        return bytes_.size();
    }
    
    size_t index_select(size_t id) const {
        return popuration_flags_.select(id);
    }
    
    size_t id_rank(size_t index) const {
        return popuration_flags_.rank(index);
    }
    
    // MARK: IO
    
    size_t size_in_bytes() const override {
        auto size = size_vec(bytes_);
        if constexpr (kIsBinaryMode)
            size += boundary_flags_.size_in_bytes();
        if constexpr (kSelectAccess)
            size += popuration_flags_.size_in_bytes();
        return size;
    }
    
    void LoadFrom(std::istream& is) override {
        bytes_ = read_vec<char>(is);
        if constexpr (kIsBinaryMode)
            boundary_flags_.Read(is);
        if constexpr (kSelectAccess)
            popuration_flags_.Read(is);
    }
    
    void StoreTo(std::ostream& os) const override {
        write_vec(bytes_, os);
        if constexpr (kIsBinaryMode)
            boundary_flags_.Write(os);
        if constexpr (kSelectAccess)
            popuration_flags_.Write(os);
    }
    
    // MARK: Show status
    
    void ShowLabels(size_t id) const {
        auto index = kSelectAccess ? index_select(id) : id;
        auto from = index >= 32 ? index - 32 : 0;
        auto to = index < bytes_.size() - 32 ? index + 32 : bytes_.size() - 1;
        std::cout << std::endl << "\tindex: " << (from + to) / 2 << ", text: " << string_view(id) << std::endl;
        std::cout << from << " ... " << index << " ... " << to << std::endl;
        for (auto i = from; i <= to; i++) {
            std::cout << (i == index ? '|' : ' ');
        }
        std::cout << std::endl;
        for (auto i = from; i <= to; i++) {
            if (i < 0 or i >= bytes_.size())
                std::cout << ' ';
            else {
                auto c =  bytes_[i];
                if (c == '\0') c = ' ';
                std::cout << c;
            }
        }
        std::cout << std::endl;
    }
    
    // MARK: Copy guard
    
    SerializedStrings() = default;
    ~SerializedStrings() = default;
    
    SerializedStrings(const SerializedStrings&) = delete;
    SerializedStrings& operator=(const SerializedStrings&) = delete;
    
    SerializedStrings(SerializedStrings&&) noexcept = default;
    SerializedStrings& operator=(SerializedStrings&&) noexcept = default;
    
private:
    Storage bytes_;
    BitVector boundary_flags_;
    SuccinctBitVector popuration_flags_;
    
};
    
}

#endif /* StringArray_hpp */

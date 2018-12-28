//
//  StringArrayBuilder.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/02/24.
//

#ifndef StringArrayBuilder_hpp
#define StringArrayBuilder_hpp

#include "SerializedStrings.hpp"
#include "sim_ds/BitVector.hpp"

namespace csd_automata {

class SerializedStringsBuilder {
public:
    using char_type = char;
    using BitVector = sim_ds::BitVector;
    
    static constexpr char_type kEndLabel = '\0';
    
    SerializedStringsBuilder(bool binary_mode) : binary_mode_(binary_mode) {}
    
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
            bytes_.emplace_back(kEndLabel);
        }
    }
    
    void SetPopuration(size_t index) {
        if (popuration_flags_.size() < index + 1)
            popuration_flags_.resize(index + 1);
        popuration_flags_[index] = true;
    }
    
    template<class Product>
    void Release(Product* product) {
        assert(Product::kIsBinaryMode == binary_mode_);
        if (Product::kIsBinaryMode != binary_mode_) {
            std::cout << "StringArray error type of binary mode!!" << std::endl;
            abort();
        }
        product->bytes_ = move(bytes_);
        if (Product::kSelectAccess) {
            popuration_flags_.Build(true);
            product->popuration_flags_ = popuration_flags_;
        }
        if (Product::kIsBinaryMode) {
            product->boundary_flags_ = boundary_flags_;
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
    
    // MARK: Copy guard
    
    SerializedStringsBuilder() = default;
    ~SerializedStringsBuilder() = default;
    
    SerializedStringsBuilder(const SerializedStringsBuilder&) = delete;
    SerializedStringsBuilder& operator=(const SerializedStringsBuilder&) = delete;
    
    SerializedStringsBuilder(SerializedStringsBuilder&&) noexcept = default;
    SerializedStringsBuilder& operator=(SerializedStringsBuilder&&) noexcept = default;
    
private:
    bool binary_mode_;
    
    std::vector<char_type> bytes_;
    BitVector boundary_flags_;
    BitVector popuration_flags_;
    
};

}

#endif /* StringArrayBuilder_hpp */

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
#include "sim_ds/SuccinctBitVector.hpp"

namespace csd_automata {

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
            bytes_.emplace_back(kEndLabel);
        }
    }
    
    void SetPopuration(size_t index) {
        if (popuration_flags_.size() < index + 1)
            popuration_flags_.resize(index + 1);
        popuration_flags_[index] = true;
    }
    
    template<bool IsBinaryMode, bool SelectAccess>
    void Release(SerializedStrings<IsBinaryMode, SelectAccess>* product) {
        using Product = SerializedStrings<IsBinaryMode, SelectAccess>;
        assert(Product::kIsBinaryMode == binary_mode_);
        if (Product::kIsBinaryMode != binary_mode_) {
            std::cout << "StringArray error type of binary mode!!" << std::endl;
            abort();
        }
        product->bytes_ = move(bytes_);
        if (Product::kSelectAccess) {
            product->popuration_flags_ = sim_ds::SuccinctBitVector<>(popuration_flags_);
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
    
};

}

#endif /* StringArrayBuilder_hpp */

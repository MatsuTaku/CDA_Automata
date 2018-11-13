//
//  StringArrayBuilder.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/02/24.
//

#ifndef StringArrayBuilder_hpp
#define StringArrayBuilder_hpp

#include "sim_ds/BitVector.hpp"
#include "SerializedStrings.hpp"

namespace csd_automata {
    
    class SerializedStringsBuilder {
    public:
        using char_type = char;
        static constexpr char_type kEndLabel = '\0';
        
    private:
        bool binary_mode_;
        
        std::vector<char_type> bytes_;
        sim_ds::BitVector boundary_flags_;
        
    public:
        SerializedStringsBuilder(bool binaryMode) : binary_mode_(binaryMode) {}
        
        bool isBinary() const {
            return binary_mode_;
        }
        
        auto operator[](size_t index) const {
            return bytes_[index];
        }
        
        auto size() const {
            return bytes_.size();
        }
        
        bool isEnd(size_t index) const {
            return binary_mode_ ? boundary_flags_[index] : bytes_[index + 1] == kEndLabel;
        }
        
        void addString(const std::string& str) {
            assert(str.size() > 0);
            for (auto c : str) {
                bytes_.emplace_back(static_cast<char_type>(c));
            }
            if (binary_mode_) {
                boundary_flags_.set(bytes_.size() - 1, true);
            } else {
                bytes_.emplace_back(kEndLabel);
            }
        }
        
        template<class _Prod>
        _Prod release() {
            _Prod product;
            assert(_Prod::_binary_mode == binary_mode_);
            if (_Prod::_binary_mode != binary_mode_) {
                std::cout << "StringArray error type of binary mode!!" << std::endl;
                abort();
            }
            product.bytes_ = move(bytes_);
            if (_Prod::_binary_mode) {
                product.boundary_flags_ = boundary_flags_;
            }
            return product;
        }
        
        SerializedStringsBuilder() = default;
        ~SerializedStringsBuilder() = default;
        
        SerializedStringsBuilder(const SerializedStringsBuilder&) = delete;
        SerializedStringsBuilder& operator=(const SerializedStringsBuilder&) = delete;
        
        SerializedStringsBuilder(SerializedStringsBuilder&&) noexcept = default;
        SerializedStringsBuilder& operator=(SerializedStringsBuilder&&) noexcept = default;
        
    };
    
}

#endif /* StringArrayBuilder_hpp */

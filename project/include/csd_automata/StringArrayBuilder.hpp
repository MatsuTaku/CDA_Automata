//
//  StringArrayBuilder.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/02/24.
//

#ifndef StringArrayBuilder_hpp
#define StringArrayBuilder_hpp

#include "sim_ds/BitVector.hpp"

namespace csd_automata {
    
    class StringArrayBuilder {
    public:
        using CType = char;
        static constexpr CType kEndLabel = '\0';
    public:
        StringArrayBuilder() = default;
        StringArrayBuilder(bool binaryMode) : binary_mode_(binaryMode) {}
        ~StringArrayBuilder() = default;
        
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
        
        void addString(const std::string &str) {
            assert(str.size() > 0);
            for (auto c : str) {
                bytes_.emplace_back(static_cast<CType>(c));
            }
            if (binary_mode_) {
                boundary_flags_.set(bytes_.size() - 1, true);
            } else {
                bytes_.push_back(kEndLabel);
            }
        }
        
        friend std::vector<CType>& bytes(StringArrayBuilder &src) {
            return src.bytes_;
        }
        
        friend sim_ds::BitVector& boundaryFlags(StringArrayBuilder &src) {
            return src.boundary_flags_;
        }
        
        StringArrayBuilder(const StringArrayBuilder&) = delete;
        StringArrayBuilder& operator=(const StringArrayBuilder&) = delete;
        
        StringArrayBuilder(StringArrayBuilder&&) noexcept = default;
        StringArrayBuilder& operator=(StringArrayBuilder&&) noexcept = default;
        
    private:
        bool binary_mode_;
        
        std::vector<CType> bytes_;
        sim_ds::BitVector boundary_flags_;
        
    };
    
}

#endif /* StringArrayBuilder_hpp */

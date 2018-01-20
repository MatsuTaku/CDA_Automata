//
//  StringArray.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/14.
//

#ifndef StringArray_hpp
#define StringArray_hpp

#include "ByteData.hpp"

#include "basic.hpp"

namespace array_fsa {
    
    class StringArray : ByteData {
        using ArrayType = std::vector<uint8_t>;
    public:
        static constexpr uint8_t kEndLabel = '\0';
        
        // MARK: - Constructor
        
        StringArray() = default;
        StringArray(std::vector<uint8_t>& strings) {
            setStringArray(strings);
        }
        
        ~StringArray() = default;
        
        bool isMatch(size_t* pos, const std::string& str, size_t strIndex) const {
            // TODO:
            uint8_t symbol = str[*pos];
            uint8_t c = bytes_[strIndex];
            while (symbol == c) {
                c = bytes_[++strIndex];
                if (++*pos >= str.size())
                    break;
                symbol = str[*pos];
            }
            return c == kEndLabel;
        }
        
        // MARK: - build
        
        void setStringArray(std::vector<uint8_t>& strings) {
            bytes_ = std::move(strings);
        }
        
        // MARK: - ByteData method
        
        size_t sizeInBytes() const override {
            return size_vec(bytes_);
        }
        
        void write(std::ostream& os) const override {
            write_vec(bytes_, os);
        }
        
        void read(std::istream& is) override {
            bytes_ = read_vec<uint8_t>(is);
        }
        
        // MARK: - Copy guard
        
        StringArray(const StringArray&) = delete;
        StringArray& operator =(const StringArray&) = delete;
        
        StringArray(StringArray&&) noexcept = default;
        StringArray& operator =(StringArray&&) noexcept = default;
        
    private:
        ArrayType bytes_;
        
    };
    
}

#endif /* StringArray_hpp */

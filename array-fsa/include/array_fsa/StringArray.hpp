//
//  StringArray.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/14.
//

#ifndef StringArray_hpp
#define StringArray_hpp

#include "ByteData.hpp"

#include "StringArrayBuilder.hpp"

#include "sim_ds/BitVector.hpp"

namespace array_fsa {
    
    template<bool BINARY = false>
    class StringArray : ByteData {
    public:
        // MARK: - Constructor
        
        StringArray() = default;
        
        explicit StringArray(StringArrayBuilder* builder) {
            if (BINARY != builder->isBinary()) {
                std::cout << "StringArray error type of binary mode!!" << std::endl;
                abort();
            }
            bytes_ = std::move(bytes(*builder));
            if (BINARY) {
                boundary_flags_ = std::move(boundaryFlags(*builder));
            }
        }
        
        explicit StringArray(std::istream &is) {
            read(is);
        }
        
        ~StringArray() = default;
        
        // MARK: - Copy guard
        
        StringArray(const StringArray&) = delete;
        StringArray& operator=(const StringArray&) = delete;
        
        StringArray(StringArray&&) noexcept = default;
        StringArray& operator=(StringArray&&) noexcept = default;
        static constexpr bool binaryMode = BINARY;
        
    public:
        using ArrayType = std::vector<uint8_t>;
        static constexpr bool kBinaryMode = BINARY;
        static constexpr uint8_t kEndLabel = '\0';
        
    public:
        // MARK: - Property
        
        uint8_t operator[](size_t index) const {
            return bytes_[index];
        }
        
        bool isMatch(size_t* pos, const std::string &str, size_t strIndex) const {
            while (*pos < str.size()) {
                if (static_cast<uint8_t>(str[*pos]) != bytes_[strIndex])
                    return false;
                if (BINARY ? boundary_flags_[strIndex] : (bytes_[strIndex + 1] == kEndLabel))
                    return true;
                ++*pos;
                ++strIndex;
            }
            return false;
        }
        
        bool isEnd(size_t index) const {
            if (BINARY)
                return boundary_flags_[index];
            else
                return bytes_[index + 1] == kEndLabel;
        }
        
        std::string string(size_t index) const {
            std::string s;
            uint8_t c = bytes_[index];
            while (c != kEndLabel) {
                s.push_back(c);
                c = bytes_[++index];
            }
            return s;
        }
        
        size_t size() const {
            return bytes_.size();
        }
        
        // MARK: - ByteData method
        
        size_t sizeInBytes() const override {
            auto size = size_vec(bytes_);
            if (BINARY)
                size += boundary_flags_.sizeInBytes();
            return size;
        }
        
        void write(std::ostream& os) const override {
            write_vec(bytes_, os);
            if (BINARY)
                boundary_flags_.write(os);
        }
        
        void read(std::istream& is) override {
            bytes_ = read_vec<uint8_t>(is);
            if (BINARY)
                boundary_flags_.read(is);
        }
        
        // MARK: - Show status
        
        void showLabels(int from, int to) const {
            for (auto i = from; i <= to; i++) {
                std::cout << (i == (from + to) / 2 ? '|' : ' ');
            }
            std::cout << std::endl;
            for (auto i = from; i <= to; i++) {
                if (i < 0 || i >= bytes_.size())
                    std::cout << ' ';
                else {
                    auto c =  bytes_[i];
                    if (c == '\0') c = ' ';
                    std::cout << c;
                }
            }
            std::cout << std::endl;
        }
        
    private:
        ArrayType bytes_;
        sim_ds::BitVector boundary_flags_;
        
    };
    
}

#endif /* StringArray_hpp */

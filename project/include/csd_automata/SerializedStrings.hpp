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

namespace csd_automata {
    
    template<bool _Binary = false>
    class SerializedStrings : IOInterface {
    public:
        using char_type = char;
        using storage = std::vector<char_type>;
        using bit_vector = sim_ds::BitVector;
        
        static constexpr bool _binary_mode = _Binary;
        static constexpr char_type _end_label = '\0';
        
        friend class SerializedStringsBuilder;
        
    private:
        storage bytes_;
        bit_vector boundary_flags_;
        
    public:
        // MARK: Constructor
        
        explicit SerializedStrings(std::istream& is) {
            read(is);
        }
        
        // MARK: Property
        
        char_type operator[](size_t index) const {
            return bytes_[index];
        }
        
        bool isBackAt(size_t index) const {
            if constexpr (_binary_mode)
                return boundary_flags_[index];
            else
                return bytes_[index + 1] == _end_label;
        }
        
        bool match(size_t* pos, const std::string& str, size_t strIndex) const {
            for (; *pos < str.size(); ++*pos, strIndex++) {
                if (static_cast<char>(str[*pos]) != bytes_[strIndex])
                    return false;
                if (isBackAt(strIndex))
                    return true;
            }
            return false;
        }
        
        std::string string(size_t index) const {
            std::string s;
            for (char c = bytes_[index]; c != _end_label; c = bytes_[++index]) {
                s.push_back(c);
            }
            return s;
        }
        
        std::basic_string_view<char_type> string_view(size_t index) const {
            size_t i;
            for (i = index; bytes_[i] != _end_label; i++);
            return std::basic_string_view<char_type>(&bytes_[index], i - index);
        }
        
        size_t size() const {
            return bytes_.size();
        }
        
        // MARK: IO
        
        size_t sizeInBytes() const override {
            auto size = size_vec(bytes_);
            if constexpr (_binary_mode)
                size += boundary_flags_.sizeInBytes();
            return size;
        }
        
        void write(std::ostream& os) const override {
            write_vec(bytes_, os);
            if constexpr (_binary_mode)
                boundary_flags_.write(os);
        }
        
        void read(std::istream& is) override {
            bytes_ = read_vec<char>(is);
            if constexpr (_binary_mode)
                boundary_flags_.read(is);
        }
        
        // MARK: Show status
        
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
        
        // MARK: Copy guard
        
        SerializedStrings() = default;
        ~SerializedStrings() = default;
        
        SerializedStrings(const SerializedStrings&) = delete;
        SerializedStrings& operator=(const SerializedStrings&) = delete;
        
        SerializedStrings(SerializedStrings&&) noexcept = default;
        SerializedStrings& operator=(SerializedStrings&&) noexcept = default;
        
        
    };
    
}

#endif /* StringArray_hpp */

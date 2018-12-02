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
    
template<bool IsBinaryMode = false>
class SerializedStrings : IOInterface {
    
    using char_type = char;
    using Storage = std::vector<char_type>;
    using BitVector = sim_ds::BitVector;
    
    static constexpr bool kIsBinaryMode = IsBinaryMode;
    static constexpr char_type kEndLabel = '\0';
    
    friend class SerializedStringsBuilder;
    
private:
    Storage bytes_;
    BitVector boundary_flags_;
    
public:
    // MARK: Constructor
    
    explicit SerializedStrings(std::istream& is) {
        Read(is);
    }
    
    // MARK: Property
    
    char_type operator[](size_t index) const {
        return bytes_[index];
    }
    
    bool is_back_at(size_t index) const {
        if constexpr (kIsBinaryMode)
            return boundary_flags_[index];
        else
            return bytes_[index + 1] == kEndLabel;
    }
    
    bool match(size_t* pos, const std::string& str, size_t str_index) const {
        for (; *pos < str.size(); ++*pos, str_index++) {
            if (static_cast<char>(str[*pos]) != bytes_[str_index])
                return false;
            if (is_back_at(str_index))
                return true;
        }
        return false;
    }
    
    std::string string(size_t index) const {
        std::string s;
        for (char c = bytes_[index]; c != kEndLabel; c = bytes_[++index]) {
            s.push_back(c);
        }
        return s;
    }
    
    std::basic_string_view<char_type> string_view(size_t index) const {
        size_t i;
        for (i = index; bytes_[i] != kEndLabel; i++);
        return std::basic_string_view<char_type>(&bytes_[index], i - index);
    }
    
    size_t size() const {
        return bytes_.size();
    }
    
    // MARK: IO
    
    size_t size_in_bytes() const override {
        auto size = size_vec(bytes_);
        if constexpr (kIsBinaryMode)
            size += boundary_flags_.size_in_bytes();
        return size;
    }
    
    void Read(std::istream& is) override {
        bytes_ = read_vec<char>(is);
        if constexpr (kIsBinaryMode)
            boundary_flags_.Read(is);
    }
    
    void Write(std::ostream& os) const override {
        write_vec(bytes_, os);
        if constexpr (kIsBinaryMode)
            boundary_flags_.Write(os);
    }
    
    // MARK: Show status
    
    void ShowLabels(int from, int to) const {
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

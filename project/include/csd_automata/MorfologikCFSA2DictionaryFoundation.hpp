//
//  MorfologikCFSA2DictionaryFoundation.hpp
//  ArrayFSA_dev
//
//  Created by 松本拓真 on 2018/09/26.
//

#ifndef MorfologikCFSA2DictionaryFoundation_hpp
#define MorfologikCFSA2DictionaryFoundation_hpp

#include "MorfologikCFSA2.hpp"
#include "sim_ds/calc.hpp"

namespace csd_automata {
    
class MorfologikCFSA2DictionaryFoundation {
    
    const size_t kNumFlags_ = 4;
    const size_t kSizeNodeSymbol_ = 1;
    const size_t kBitsUpperNodeWords_ = 4;
    
    size_t element_words_lower_size_ = 4;
    std::vector<uint8_t> bytes_;
    
public:
    using FsaSource = MorfologikCFSA2;
    
    static std::string name() {
        return FsaSource::name();
    }
    
    MorfologikCFSA2DictionaryFoundation(const FsaSource& set);
    
    MorfologikCFSA2DictionaryFoundation(std::istream &is) {
        Read(is);
    }
    
    // MARK: Transition parameters
    
    uint8_t get_trans_symbol(size_t trans) const {
        return bytes_[offset_symbol_(trans)];
    }
    
    size_t get_trans_words(size_t trans) const {
        size_t words = 0;
        auto size = is_words_large(trans) ? 1 + element_words_lower_size_ : 1;
        std::memcpy(&words, &bytes_[offset_upper_words_(trans)], size);
        return words >> kNumFlags_;
    }
    
    size_t get_goto(size_t trans) const {
        assert(!is_next_set(trans));
        auto offset = offset_address_(trans);
        auto byte = bytes_[offset];
        size_t ret = static_cast<size_t>(byte & 0x7F);
        for (size_t shift = 7; byte & 0x80; shift += 7) {
            byte = bytes_[++offset];
            ret |= (byte & 0x7F) << shift;
        }
        return ret;
    }
    
    bool is_final_trans(size_t trans) const {
        return static_cast<bool>(bytes_[offset_flags_(trans)] & 1U);
    }
    
    bool is_last_trans(size_t trans) const {
        return static_cast<bool>(bytes_[offset_flags_(trans)] & 2U);
    }
    
    bool is_next_set(size_t trans) const {
        return static_cast<bool>(bytes_[offset_flags_(trans)] & 4U);
    }
    
    bool is_words_large(size_t trans) const {
        return static_cast<bool>(bytes_[offset_flags_(trans)] & 8U);
    }
    
    // MARK: Functionals
    
    size_t get_first_trans(size_t state) const {
        return state;
    }
    
    size_t skip_trans(size_t trans) const {
        return offset_address_(trans) + get_addr_size(trans);
    }
    
    size_t get_next_trans(size_t trans) const {
        return is_last_trans(trans) ? 0 : skip_trans(trans);
    }
    
    size_t get_target_state(size_t trans) const {
        if (is_next_set(trans)) {
            for (; !is_last_trans(trans); trans = get_next_trans(trans));
            return skip_trans(trans);
        } else {
            return get_goto(trans);
        }
    }
    
    size_t get_root_state() const {
        return get_target_state(get_first_trans(0));
    }
    
    size_t get_trans(size_t state, uint8_t symbol) const {
        for (auto t = get_first_trans(state); t != 0; t = get_next_trans(t)) {
            if (get_trans_symbol(t) == symbol)
                return t;
        }
        return 0;
    }
    
    // MARK: FSA functions
    
    size_t num_trans() const {
        size_t ret = 0;
        for (size_t i = 0; i < bytes_.size(); i = skip_trans(i)) {
            ++ret;
        }
        return ret;
    }
    
    size_t size_in_bytes() const {
        auto size = 0;
        size += sizeof(element_words_lower_size_);
        size += size_vec(bytes_);
        return size;
    }
    
    void PrintForDebug(std::ostream& os) const {
        using std::endl;
        os << "\tLb\tFi\tLa\tNe\tLW\tWo\tAd" << endl;
        
        size_t i = 0;
        while (i < bytes_.size()) {
            char c = get_trans_symbol(i);
            if (c == '\r') {
                c = '?';
            }
            os << i << "\t"
            << c << "\t"
            << bool_str(is_final_trans(i)) << "\t"
            << bool_str(is_last_trans(i)) << "\t"
            << bool_str(is_next_set(i)) << "\t"
            << bool_str(is_words_large(i)) << "\t"
            << get_trans_words(i) << "\t";
            if (!is_next_set(i)) {
                os << get_goto(i);
            }
            os << endl;
            i = skip_trans(i);
        }
    }
    
    // MARK: IO
    
    void Read(std::istream &is) {
        element_words_lower_size_ = read_val<size_t>(is);
        bytes_ = read_vec<uint8_t>(is);
    }
    
    void Write(std::ostream &os) const {
        write_val(element_words_lower_size_, os);
        write_vec(bytes_, os);
    }
    
    MorfologikCFSA2DictionaryFoundation() = default;
    ~MorfologikCFSA2DictionaryFoundation() = default;
    
    MorfologikCFSA2DictionaryFoundation(const MorfologikCFSA2DictionaryFoundation &) = delete;
    MorfologikCFSA2DictionaryFoundation &operator=(const MorfologikCFSA2DictionaryFoundation &) = delete;
    
    MorfologikCFSA2DictionaryFoundation(MorfologikCFSA2DictionaryFoundation &&) = default;
    MorfologikCFSA2DictionaryFoundation &operator=(MorfologikCFSA2DictionaryFoundation &&) = default;
    
private:
    // MARK: Offsets
    
    size_t offset_symbol_(size_t trans) const {
        assert(trans < bytes_.size());
        return trans;
    }
    
    size_t offset_flags_(size_t trans) const {
        assert(trans < bytes_.size());
        return offset_symbol_(trans) + kSizeNodeSymbol_;
    }
    
    size_t offset_upper_words_(size_t trans) const {
        assert(trans < bytes_.size());
        return offset_flags_(trans);
    }
    
    size_t offset_lower_words(size_t trans) const {
        assert(trans < bytes_.size());
        return offset_upper_words_(trans) + 1;
    }
    
    size_t offset_address_(size_t trans) const {
        assert(trans < bytes_.size());
        return offset_lower_words(trans) + (is_words_large(trans) ? element_words_lower_size_ : 0);
    }
    
    size_t get_addr_size(size_t trans) const {
        assert(trans < bytes_.size());
        if (is_next_set(trans))
            return 0;
        auto offset = offset_address_(trans);
        auto size = 1;
        uint8_t byte = bytes_[offset];
        while (byte & 0x80) {
            byte = bytes_[offset + size++];
        }
        return size;
    }
    
};


MorfologikCFSA2DictionaryFoundation::MorfologikCFSA2DictionaryFoundation(const MorfologikCFSA2DictionaryFoundation::FsaSource& origin) {
    struct Trans {
        size_t words = 0;
        size_t offset = 0;
    };
    std::unordered_map<size_t, Trans> transes;
    
    const std::function<size_t(size_t)> dfs = [&origin, &dfs, &transes](size_t state) {
        size_t count_words = 0;
        for (auto t = origin.get_first_trans(state); t != 0; t = origin.get_next_trans(t)) {
            auto it = transes.find(t);
            if (it != transes.end()) {
                count_words += it->second.words;
            } else {
                size_t word = dfs(origin.get_target_state(t));
                if (origin.is_final_trans(t))
                    word++;
                transes[t].words = word;
                count_words += word;
            }
        }
        
        return count_words;
    };
    
    auto total_words = dfs(origin.get_root_state());
    element_words_lower_size_ = sim_ds::calc::SizeFitsInBytes(total_words >> kBitsUpperNodeWords_);
    
    for (size_t s = 0; s < origin.bytes_.size(); s = origin.skip_trans_(s)) {
        transes[s].offset = std::numeric_limits<size_t>::max();
    }
    
    // Transpose fsa
    bool is_value_changed = true;
    while (is_value_changed) {
        is_value_changed = false;
        
        bytes_.resize(0);
        for (size_t s = 0, offset = 0; s < origin.bytes_.size(); s = origin.skip_trans_(s)) {
            auto& trans = transes[s];
            
            assert(trans.offset >= offset); // Every time target offset is decreasing repeats.
            if (trans.offset != offset) {
                is_value_changed = true;
                trans.offset = offset;
            }
            
            // Enumerate parameters
            size_t flags_and_words = origin.is_final_trans(s) | (origin.is_last_trans(s) << 1) | (origin.is_next_set_(s) << 2);
            size_t words = trans.words;
            flags_and_words |= (words << kNumFlags_);
            auto sizePW = 1;
            bool is_large_words = words >> kBitsUpperNodeWords_;
            if (is_large_words) {
                flags_and_words |= 0x08;
                sizePW += element_words_lower_size_;
            }
            
            auto node_size = kSizeNodeSymbol_ + sizePW;
            bytes_.resize(offset + node_size);
            uint8_t symbol = origin.get_trans_symbol(s);
            // Transfer symbol
            bytes_[offset] = symbol;
            // Transfer flags
            std::memcpy(&bytes_[offset + kSizeNodeSymbol_], &flags_and_words, sizePW);
            
            // Transfer address
            if (!origin.is_next_set_(s)) {
                size_t value = transes[origin.get_target_state(s)].offset;
                while (value > 0x7F) {
                    bytes_.emplace_back(0x80 | (value & 0x7F));
                    node_size++;
                    value >>= 7;
                }
                bytes_.emplace_back(static_cast<uint8_t>(value));
                node_size++;
            }
            
            offset += node_size;
            
        }
        std::cerr << transes[origin.get_dest_state_offset_(0)].offset << std::endl;
    }
    
//    PrintForDebug(std::cout);
}
    
}

#endif /* MorfologikCFSA2DictionaryFoundation_hpp */

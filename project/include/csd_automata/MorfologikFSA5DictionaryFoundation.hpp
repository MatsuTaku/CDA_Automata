//
//  MorfologikFSA5DictionaryFoundation.hpp
//  ArrayFSA_dev
//
//  Created by 松本拓真 on 2018/09/26.
//

#ifndef MorfologikFSA5DictionaryFoundation_hpp
#define MorfologikFSA5DictionaryFoundation_hpp

#include "MorfologikFSA5.hpp"
#include "sim_ds/calc.hpp"

namespace csd_automata {
    
class MorfologikFSA5DictionaryFoundation {
    
    size_t node_data_length_ = 0;
    
    const size_t kNumParams_ = 4;
    const size_t kSizeElementSymbol_ = 1;
    const size_t kSizeElementWordsUpperBits_ = 4;
    size_t element_words_lower_size_ = 4;
    size_t element_address_size_ = 4;
    
    std::vector<uint8_t> bytes_;
    
public:
    using FsaSource = MorfologikFSA5;
    
    static std::string name() {
        return FsaSource::name();
    }
    
    MorfologikFSA5DictionaryFoundation(const FsaSource &set);
    
    MorfologikFSA5DictionaryFoundation(std::istream &is) {
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
        return words >> kNumParams_;
    }
    
    size_t get_goto(size_t trans) const {
        assert(!is_next_set(trans));
        size_t ret = 0;
        std::memcpy(&ret, &bytes_[offset_address_(trans)], element_address_size_);
        return ret;
    }
    
    bool is_final_trans(size_t trans) const {
        return static_cast<bool>(bytes_[offset_params_(trans)] & 1U);
    }
    
    bool is_last_trans(size_t trans) const {
        return static_cast<bool>(bytes_[offset_params_(trans)] & 2U);
    }
    
    bool is_next_set(size_t trans) const {
        return static_cast<bool>(bytes_[offset_params_(trans)] & 4U);
    }
    
    bool is_words_large(size_t trans) const {
        return static_cast<bool>(bytes_[offset_params_(trans)] & 8U);
    }
    
    // MARK: Functionals
    
    size_t get_first_trans(size_t state) const {
        return node_data_length_ + state;
    }
    
    size_t skip_trans(size_t trans) const {
        return offset_address_(trans) + (is_next_set(trans) ? 0 : element_address_size_);
    }
    
    size_t get_next_trans(size_t trans) const {
        return is_last_trans(trans) ? 0 : skip_trans(trans);
    }
    
    size_t get_target_state(size_t trans) const {
        return is_next_set(trans) ? skip_trans(trans) : get_goto(trans);
    }
    
    size_t get_root_state() const {
        auto epsilonState = skip_trans(get_first_trans(0));
        return get_target_state(get_first_trans(epsilonState));
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
        size += sizeof(node_data_length_);
        size += sizeof(element_words_lower_size_);
        size += sizeof(element_address_size_);
        size += size_vec(bytes_);
        return size;
    }
    
    void PrintForDebug(std::ostream& os) const {
        using std::endl;
        os << "\tLB\tF\tL\tN\tLW\tWO\tAD" << endl;
        
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
        node_data_length_ = read_val<size_t>(is);
        element_words_lower_size_ = read_val<size_t>(is);
        element_address_size_ = read_val<size_t>(is);
        bytes_ = read_vec<uint8_t>(is);
    }
    
    void Write(std::ostream &os) const {
        write_val(node_data_length_, os);
        write_val(element_words_lower_size_, os);
        write_val(element_address_size_, os);
        write_vec(bytes_, os);
    }
    
    MorfologikFSA5DictionaryFoundation() = default;
    ~MorfologikFSA5DictionaryFoundation() = default;
    
    MorfologikFSA5DictionaryFoundation(const MorfologikFSA5DictionaryFoundation &) = delete;
    MorfologikFSA5DictionaryFoundation &operator=(const MorfologikFSA5DictionaryFoundation &) = delete;
    
    MorfologikFSA5DictionaryFoundation(MorfologikFSA5DictionaryFoundation &&) = default;
    MorfologikFSA5DictionaryFoundation &operator=(MorfologikFSA5DictionaryFoundation &&) = default;
    
private:
    // MARK: Offsets
    
    size_t offset_symbol_(size_t trans) const {
        assert(trans < bytes_.size());
        return trans;
    }
    
    size_t offset_params_(size_t trans) const {
        assert(trans < bytes_.size());
        return offset_symbol_(trans) + kSizeElementSymbol_;
    }
    
    size_t offset_upper_words_(size_t trans) const {
        assert(trans < bytes_.size());
        return offset_params_(trans);
    }
    
    size_t offset_lower_words(size_t trans) const {
        assert(trans < bytes_.size());
        return offset_upper_words_(trans) + 1;
    }
    
    size_t offset_address_(size_t trans) const {
        assert(trans < bytes_.size());
        return offset_lower_words(trans) + (is_words_large(trans) ? element_words_lower_size_ : 0);
    }
    
};


MorfologikFSA5DictionaryFoundation::MorfologikFSA5DictionaryFoundation(const MorfologikFSA5DictionaryFoundation::FsaSource& set) {
    node_data_length_ = set.node_data_length_;
    
    struct Node {
        size_t words = 0;
        size_t destination = 0;
        std::vector<size_t> parents;
    };
    
    std::unordered_map<size_t, Node> nodes;
    
    const std::function<size_t(size_t)> dfs = [&set, &dfs, &nodes](size_t state) {
        size_t count_words = 0;
        for (auto t = set.get_first_trans(state); t != 0; t = set.get_next_trans(t)) {
            auto it = nodes.find(t);
            if (it != nodes.end()) {
                count_words += it->second.words;
            } else {
                size_t words = dfs(set.get_target_state(t));
                if (set.is_final_trans(t))
                    words++;
                nodes[t].words = words;
                count_words += words;
            }
        }
        
        return count_words;
    };
    
    auto total_words = dfs(set.get_root_state());
    element_words_lower_size_ = sim_ds::calc::SizeFitsInBytes(total_words >> kSizeElementWordsUpperBits_);
    
    auto upper_new_size = set.bytes_.size() + set.get_num_trans() * element_words_lower_size_;
    element_address_size_ = sim_ds::calc::SizeFitsInBytes(upper_new_size);
    
    for (size_t s = 0, t = 0; s < set.bytes_.size(); s = set.skip_trans_(s)) {
        size_t params_and_words = set.bytes_[s + 1] & 0x07;
        size_t words = nodes[s].words;
        params_and_words |= (words << kNumParams_);
        auto size_pw = 1;
        bool is_large_words = words >= (1ULL << kSizeElementWordsUpperBits_);
        if (is_large_words) {
            params_and_words |= 8U;
            size_pw += element_words_lower_size_;
        }
        auto elementSize = kSizeElementSymbol_ + size_pw + (set.is_next_set_(s) ? 0 : element_address_size_);
        
        // Copy symbol, params and words
        bytes_.resize(bytes_.size() + elementSize);
        auto symbol = set.get_trans_symbol(s);
        std::memcpy(&bytes_[offset_symbol_(t)], &symbol, kSizeElementSymbol_);
        std::memcpy(&bytes_[offset_params_(t)], &params_and_words, size_pw);
        
        nodes[s].destination = t;
        
        t += elementSize;
    }
    
    for (size_t t = 0; t < set.bytes_.size(); t = set.skip_trans_(t)) {
        nodes[set.get_target_state(t)].parents.emplace_back(t);
    }
    
    for (size_t state = 0; state < set.bytes_.size(); state = set.skip_trans_(state)) {
        for (auto parent : nodes[state].parents) {
            if (set.is_next_set_(parent))
                continue;
            // Copy target
            std::memcpy(&bytes_[offset_address_(nodes[parent].destination)], &nodes[state].destination, element_address_size_);
        }
        
        while (!set.is_last_trans(state))
            state = set.skip_trans_(state);
    }
}
    
}

#endif /* MorfologikFSA5DictionaryFoundation_hpp */

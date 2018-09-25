//
//  MorfologikFSA5Dictionary.hpp
//  ArrayFSA_dev
//
//  Created by 松本拓真 on 2018/09/24.
//

#ifndef MorfologikFSA5Dictionary_hpp
#define MorfologikFSA5Dictionary_hpp

#include "MorfologikFSA5.hpp"
#include "sim_ds/Calc.hpp"

namespace array_fsa {
    
    class MorfologikFSA5Dictionary {
    public:
        MorfologikFSA5Dictionary(const MorfologikFSA5 &set);
        
        MorfologikFSA5Dictionary(std::istream &is) {
            read(is);
        }
        
        ~MorfologikFSA5Dictionary() = default;
        
        MorfologikFSA5Dictionary(const MorfologikFSA5Dictionary &) = delete;
        MorfologikFSA5Dictionary &operator=(const MorfologikFSA5Dictionary &) = delete;
        
        MorfologikFSA5Dictionary(MorfologikFSA5Dictionary &&) = default;
        MorfologikFSA5Dictionary &operator=(MorfologikFSA5Dictionary &&) = default;
        
        // MARK: String-Dictionary's functions
        
        static std::string name() {
            return "MorfologikFSA5Dictionary";
        }
        
        template<typename S>
        bool isMember(const S &str) const;
        
        template<typename S>
        size_t lookup(const S &str) const;
        
        std::string access(size_t id) const;
        
        size_t getNumTrans() const {
            size_t ret = 0;
            for (size_t i = 0; i < bytes_.size(); i = skipTrans_(i)) {
                ++ret;
            }
            return ret;
        }
        
        size_t sizeInBytes() const {
            auto size = 0;
            size += sizeof(node_data_length_);
            size += sizeof(element_words_size_);
            size += sizeof(element_params_size_);
            size += size_vec(bytes_);
            return size;
        }
        
        void showStatus(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << getNumTrans() << endl;
            os << "size:   " << sizeInBytes() << endl;
        }
        
        void printForDebug(std::ostream& os) const {
            using std::endl;
            os << "\tLB\tF\tL\tN\tWO\tAD" << endl;
            
            size_t i = 0;
            while (i < bytes_.size()) {
                if (i < 105755192) {
                    i = skipTrans_(i);
                    continue;
                }
                char c = getTransSymbol_(i);
                if (c == '\r') {
                    c = '?';
                }
                os << i << "\t"
                << c << "\t"
                << bool_str(isFinalTrans_(i)) << "\t"
                << bool_str(isLastTrans_(i)) << "\t"
                << bool_str(isNextSet_(i)) << "\t"
                << getTransWords_(i) << "\t";
                if (!isNextSet_(i)) {
                    os << getGoto_(i);
                }
                os << endl;
                i = skipTrans_(i);
            }
        }
        
        // MARK: IO
        
        void read(std::istream &is) {
            node_data_length_ = read_val<size_t>(is);
            element_words_size_ = read_val<size_t>(is);
            element_params_size_ = read_val<size_t>(is);
            bytes_ = read_vec<uint8_t>(is);
        }
        
        void write(std::ostream &os) const {
            write_val(node_data_length_, os);
            write_val(element_words_size_, os);
            write_val(element_params_size_, os);
            write_vec(bytes_, os);
        }
        
    private:
        size_t node_data_length_ = 0;
        const size_t kNumParams_ = 3;
        const size_t kElementSymbolSize_ = 1;
        size_t element_words_size_ = 4;
        size_t element_params_size_ = 4;
        std::vector<uint8_t> bytes_;
        
        // MARK: Transition parameters
        
        uint8_t getTransSymbol_(size_t trans) const {
            return bytes_[trans];
        }
        
        size_t getTransWords_(size_t trans) const {
            size_t words = 0;
            std::memcpy(&words, &bytes_[trans + kElementSymbolSize_], element_words_size_);
            return words;
        }
        
        bool isFinalTrans_(size_t trans) const {
            return (bytes_[trans + kElementSymbolSize_ + element_words_size_] & 1) != 0;
        }
        
        bool isLastTrans_(size_t trans) const {
            return (bytes_[trans + kElementSymbolSize_ + element_words_size_] & 2) != 0;
        }
        
        bool isNextSet_(size_t trans) const {
            return (bytes_[trans + kElementSymbolSize_ + element_words_size_] & 4) != 0;
        }
        
        // MARK: Functionals
        
        size_t getGoto_(size_t trans) const {
            size_t ret = 0;
            std::memcpy(&ret, &bytes_[trans + kElementSymbolSize_ + element_words_size_], element_params_size_);
            return ret >> kNumParams_;
        }
        
        size_t getFirstTrans_(size_t state) const {
            return node_data_length_ + state;
        }
        
        size_t skipTrans_(size_t trans) const {
            return trans + kElementSymbolSize_ + element_words_size_ + (isNextSet_(trans) ? 1 : element_params_size_);
        }
        
        size_t getNextTrans_(size_t trans) const {
            return isLastTrans_(trans) ? 0 : skipTrans_(trans);
        }
        
        size_t getTargetState_(size_t trans) const {
            return isNextSet_(trans) ? skipTrans_(trans) : getGoto_(trans);
        }
        
        size_t getRootState_() const {
            auto epsilonState = skipTrans_(getFirstTrans_(0));
            return getTargetState_(getFirstTrans_(epsilonState));
        }
        
        size_t getTrans(size_t state, uint8_t symbol) const {
            for (auto t = getFirstTrans_(state); t != 0; t = getNextTrans_(t)) {
                if (getTransSymbol_(t) == symbol)
                    return t;
            }
            return 0;
        }
        
    };
    
    
    MorfologikFSA5Dictionary::MorfologikFSA5Dictionary(const MorfologikFSA5 &set) {
        node_data_length_ = set.node_data_length_;
        
        std::map<size_t, size_t> words;
        
        const std::function<size_t(size_t)> dfs = [&set, &dfs, &words](size_t state) {
            size_t wordsCount = 0;
            for (auto t = set.get_first_trans(state); t != 0; t = set.get_next_trans(t)) {
                auto it = words.find(t);
                if (it != words.end()) {
                    wordsCount += it->second;
                } else {
                    size_t word = dfs(set.get_target_state(t));
                    if (set.is_final_trans(t))
                        word++;
                    words[t] = word;
                    wordsCount += word;
                }
            }
            
            return wordsCount;
        };
        
        auto allWords = dfs(set.get_root_state());
        element_words_size_ = sim_ds::Calc::sizeFitInBytes(allWords);
        
        auto newSize = set.bytes_.size() + set.get_num_trans() * element_words_size_;
        element_params_size_ = sim_ds::Calc::sizeFitInBytes(newSize << 3);
        
        bytes_.resize(newSize);
        std::map<size_t, size_t> mappings;
        for (size_t s = 0, t = 0; s < set.bytes_.size(); s = set.skip_trans_(s)) {
            // Copy symbol
            std::memcpy(&bytes_[t], &set.bytes_[s], kElementSymbolSize_);
            // Set words
            std::memcpy(&bytes_[t + kElementSymbolSize_], &words[s], element_words_size_);
            // Copy params
            std::memcpy(&bytes_[t + kElementSymbolSize_ + element_words_size_], &set.bytes_[s + 1], 1);
            
            mappings[s] = t;
            
            auto paramLen = set.is_next_set_(s) ? 1 : element_params_size_;
            t += kElementSymbolSize_ + element_words_size_ + paramLen;
        }
        
        std::map<size_t, std::vector<size_t>> parents;
        for (size_t t = 0; t < set.bytes_.size(); t = set.skip_trans_(t)) {
            parents[set.get_target_state(t)].push_back(t);
        }
        
        for (size_t state = 0; state < set.bytes_.size(); state = set.skip_trans_(state)) {
            for (auto parent : parents[state]) {
                if (set.is_next_set_(parent))
                    continue;
                auto paramsOff = mappings[parent] + kElementSymbolSize_ + element_words_size_;
                size_t params = bytes_[paramsOff];
                params = (params & 0x07) | (mappings[state] << 3);
                // Copy target
                std::memcpy(&bytes_[paramsOff], &params, element_params_size_);
            }
            
            while (!set.is_last_trans(state))
                state = set.skip_trans_(state);
        }
    }
    
    template<typename S>
    inline bool MorfologikFSA5Dictionary::isMember(const S &str) const {
        size_t state = getRootState_(), arc = 0;
        for (uint8_t c : str) {
            arc = getTrans(state, c);
            if (arc == 0) {
                std::cerr << "Error not membered: " << str << std::endl;
                return false;
            }
            
            state = getTargetState_(arc);
        }
        
        return isFinalTrans_(arc);
    }
    
    template<typename S>
    inline size_t MorfologikFSA5Dictionary::lookup(const S &str) const {
        size_t words = 0;
        
        size_t state = getRootState_(), arc = 0;
        for (uint8_t c : str) {
            for (arc = getFirstTrans_(state); arc != 0 && getTransSymbol_(arc) != c; arc = getNextTrans_(arc)) {
                words += getTransWords_(arc);
            }
            if (arc == 0) {
                std::cerr << "Error not membered: " << str << std::endl;
                return -1;
            }
            
            if (isFinalTrans_(arc))
                words++;
            
            state = getTargetState_(arc);
        }
        
        return isFinalTrans_(arc) ? words : -1;
    }
    
    inline std::string MorfologikFSA5Dictionary::access(size_t id) const {
        std::string str = "";
        
        size_t counter = id;
        for (size_t state = getRootState_(), arc = 0; counter > 0; state = getTargetState_(arc)) {
            for (arc = getFirstTrans_(state); arc != 0 && counter > 0; arc = getNextTrans_(arc)) {
                auto words = getTransWords_(arc);
                if (words < counter) {
                    counter -= words;
                } else {
                    if (isFinalTrans_(arc))
                        counter--;
                    str += getTransSymbol_(arc);
                    break;
                }
            }
            if (arc == 0) {
                std::cerr << "Error not stored id: " << id << std::endl;
                return "";
            }
        }
        
        return str;
    }
    
}

#endif /* MorfologikFSA5Dictionary_hpp */

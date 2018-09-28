//
//  MorfologikFSA5DictionaryFoundation.hpp
//  ArrayFSA_dev
//
//  Created by 松本拓真 on 2018/09/26.
//

#ifndef MorfologikFSA5DictionaryFoundation_hpp
#define MorfologikFSA5DictionaryFoundation_hpp

#include "MorfologikFSA5.hpp"
#include "sim_ds/Calc.hpp"

namespace array_fsa {
    
    class MorfologikFSA5DictionaryFoundation {
    public:
        using FSAType = MorfologikFSA5;
        
        MorfologikFSA5DictionaryFoundation(const FSAType &set);
        
        MorfologikFSA5DictionaryFoundation(std::istream &is) {
            read(is);
        }
        
        MorfologikFSA5DictionaryFoundation() = default;
        
        MorfologikFSA5DictionaryFoundation(const MorfologikFSA5DictionaryFoundation &) = delete;
        MorfologikFSA5DictionaryFoundation &operator=(const MorfologikFSA5DictionaryFoundation &) = delete;
        
        MorfologikFSA5DictionaryFoundation(MorfologikFSA5DictionaryFoundation &&) = default;
        MorfologikFSA5DictionaryFoundation &operator=(MorfologikFSA5DictionaryFoundation &&) = default;
        
        // MARK: Transition parameters
        
        uint8_t getTransSymbol(size_t trans) const {
            return bytes_[offsetSymbol_(trans)];
        }
        
        size_t getTransWords(size_t trans) const {
            size_t words = 0;
            auto size = isWordsLarge(trans) ? 1 + element_words_lower_size_ : 1;
            std::memcpy(&words, &bytes_[offsetUpperWords_(trans)], size);
            return words >> kNumParams_;
        }
        
        size_t getGoto(size_t trans) const {
            assert(!isNextSet(trans));
            size_t ret = 0;
            std::memcpy(&ret, &bytes_[offsetAddress_(trans)], element_address_size_);
            return ret;
        }
        
        bool isFinalTrans(size_t trans) const {
            return (bytes_[offsetParams_(trans)] & 1U) != 0;
        }
        
        bool isLastTrans(size_t trans) const {
            return (bytes_[offsetParams_(trans)] & 2U) != 0;
        }
        
        bool isNextSet(size_t trans) const {
            return (bytes_[offsetParams_(trans)] & 4U) != 0;
        }
        
        bool isWordsLarge(size_t trans) const {
            return (bytes_[offsetParams_(trans)] & 8U) != 0;
        }
        
        // MARK: Functionals
        
        size_t getFirstTrans(size_t state) const {
            return node_data_length_ + state;
        }
        
        size_t skipTrans(size_t trans) const {
            return offsetAddress_(trans) + (isNextSet(trans) ? 0 : element_address_size_);
        }
        
        size_t getNextTrans(size_t trans) const {
            return isLastTrans(trans) ? 0 : skipTrans(trans);
        }
        
        size_t getTargetState(size_t trans) const {
            return isNextSet(trans) ? skipTrans(trans) : getGoto(trans);
        }
        
        size_t getRootState() const {
            auto epsilonState = skipTrans(getFirstTrans(0));
            return getTargetState(getFirstTrans(epsilonState));
        }
        
        size_t getTrans(size_t state, uint8_t symbol) const {
            for (auto t = getFirstTrans(state); t != 0; t = getNextTrans(t)) {
                if (getTransSymbol(t) == symbol)
                    return t;
            }
            return 0;
        }
        
        // MARK: FSA functions
        
        size_t numTrans() const {
            size_t ret = 0;
            for (size_t i = 0; i < bytes_.size(); i = skipTrans(i)) {
                ++ret;
            }
            return ret;
        }
        
        size_t sizeInBytes() const {
            auto size = 0;
            size += sizeof(node_data_length_);
            size += sizeof(element_words_lower_size_);
            size += sizeof(element_address_size_);
            size += size_vec(bytes_);
            return size;
        }
        
        void printForDebug(std::ostream& os) const {
            using std::endl;
            os << "\tLB\tF\tL\tN\tLW\tWO\tAD" << endl;
            
            size_t i = 0;
            while (i < bytes_.size()) {
                char c = getTransSymbol(i);
                if (c == '\r') {
                    c = '?';
                }
                os << i << "\t"
                << c << "\t"
                << bool_str(isFinalTrans(i)) << "\t"
                << bool_str(isLastTrans(i)) << "\t"
                << bool_str(isNextSet(i)) << "\t"
                << bool_str(isWordsLarge(i)) << "\t"
                << getTransWords(i) << "\t";
                if (!isNextSet(i)) {
                    os << getGoto(i);
                }
                os << endl;
                i = skipTrans(i);
            }
        }
        
        // MARK: IO
        
        void read(std::istream &is) {
            node_data_length_ = read_val<size_t>(is);
            element_words_lower_size_ = read_val<size_t>(is);
            element_address_size_ = read_val<size_t>(is);
            bytes_ = read_vec<uint8_t>(is);
        }
        
        void write(std::ostream &os) const {
            write_val(node_data_length_, os);
            write_val(element_words_lower_size_, os);
            write_val(element_address_size_, os);
            write_vec(bytes_, os);
        }
        
    private:
        size_t node_data_length_ = 0;
        
        const size_t kNumParams_ = 4;
        const size_t kElementSymbolSize_ = 1;
        const size_t kElementWordsUpperBitsSize_ = 4;
        size_t element_words_lower_size_ = 4;
        size_t element_address_size_ = 4;
        
        std::vector<uint8_t> bytes_;
        
        // MARK: Offsets
        
        size_t offsetSymbol_(size_t trans) const {
            assert(trans < bytes_.size());
            return trans;
        }
        
        size_t offsetParams_(size_t trans) const {
            assert(trans < bytes_.size());
            return offsetSymbol_(trans) + kElementSymbolSize_;
        }
        
        size_t offsetUpperWords_(size_t trans) const {
            assert(trans < bytes_.size());
            return offsetParams_(trans);
        }
        
        size_t offsetLowerWords(size_t trans) const {
            assert(trans < bytes_.size());
            return offsetUpperWords_(trans) + 1;
        }
        
        size_t offsetAddress_(size_t trans) const {
            assert(trans < bytes_.size());
            return offsetLowerWords(trans) + (isWordsLarge(trans) ? element_words_lower_size_ : 0);
        }
        
    };
    
    
    MorfologikFSA5DictionaryFoundation::MorfologikFSA5DictionaryFoundation(const MorfologikFSA5 &set) {
        node_data_length_ = set.node_data_length_;
        
        struct Node {
            size_t words = 0;
            size_t destination = 0;
            std::vector<size_t> parents;
        };
        
        std::map<size_t, Node> nodes;
        
        const std::function<size_t(size_t)> dfs = [&set, &dfs, &nodes](size_t state) {
            size_t wordsCount = 0;
            for (auto t = set.get_first_trans(state); t != 0; t = set.get_next_trans(t)) {
                auto it = nodes.find(t);
                if (it != nodes.end()) {
                    wordsCount += it->second.words;
                } else {
                    size_t words = dfs(set.get_target_state(t));
                    if (set.is_final_trans(t))
                        words++;
                    nodes[t].words = words;
                    wordsCount += words;
                }
            }
            
            return wordsCount;
        };
        
        auto allWords = dfs(set.get_root_state());
        element_words_lower_size_ = sim_ds::Calc::sizeFitInBytes(allWords >> kElementWordsUpperBitsSize_);
        
        auto upperNewSize = set.bytes_.size() + set.get_num_trans() * element_words_lower_size_;
        element_address_size_ = sim_ds::Calc::sizeFitInBytes(upperNewSize);
        
        for (size_t s = 0, t = 0; s < set.bytes_.size(); s = set.skip_trans_(s)) {
            size_t paramsAndWords = set.bytes_[s + 1] & 0x07;
            size_t words = nodes[s].words;
            paramsAndWords |= (words << kNumParams_);
            auto sizePW = 1;
            bool isLargeWords = words >= (1ULL << kElementWordsUpperBitsSize_);
            if (isLargeWords) {
                paramsAndWords |= 8U;
                sizePW += element_words_lower_size_;
            }
            auto elementSize = kElementSymbolSize_ + sizePW + (set.is_next_set_(s) ? 0 : element_address_size_);
            
            // Copy symbol, params and words
            bytes_.resize(bytes_.size() + elementSize);
            auto symbol = set.get_trans_symbol(s);
            std::memcpy(&bytes_[offsetSymbol_(t)], &symbol, kElementSymbolSize_);
            std::memcpy(&bytes_[offsetParams_(t)], &paramsAndWords, sizePW);
            
            nodes[s].destination = t;
            
            t += elementSize;
        }
        
//        std::map<size_t, std::vector<size_t>> parents;
        for (size_t t = 0; t < set.bytes_.size(); t = set.skip_trans_(t)) {
            nodes[set.get_target_state(t)].parents.emplace_back(t);
        }
        
        for (size_t state = 0; state < set.bytes_.size(); state = set.skip_trans_(state)) {
            for (auto parent : nodes[state].parents) {
                if (set.is_next_set_(parent))
                    continue;
                // Copy target
                std::memcpy(&bytes_[offsetAddress_(nodes[parent].destination)], &nodes[state].destination, element_address_size_);
            }
            
            while (!set.is_last_trans(state))
                state = set.skip_trans_(state);
        }
    }
    
}

#endif /* MorfologikFSA5DictionaryFoundation_hpp */

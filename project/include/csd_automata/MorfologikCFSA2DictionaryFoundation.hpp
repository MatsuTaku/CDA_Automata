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
    public:
        using FSAType = MorfologikCFSA2;
        
        MorfologikCFSA2DictionaryFoundation(const FSAType &set);
        
        MorfologikCFSA2DictionaryFoundation(std::istream &is) {
            read(is);
        }
        
        MorfologikCFSA2DictionaryFoundation() = default;
        
        MorfologikCFSA2DictionaryFoundation(const MorfologikCFSA2DictionaryFoundation &) = delete;
        MorfologikCFSA2DictionaryFoundation &operator=(const MorfologikCFSA2DictionaryFoundation &) = delete;
        
        MorfologikCFSA2DictionaryFoundation(MorfologikCFSA2DictionaryFoundation &&) = default;
        MorfologikCFSA2DictionaryFoundation &operator=(MorfologikCFSA2DictionaryFoundation &&) = default;
        
        // MARK: Transition parameters
        
        uint8_t getTransSymbol(size_t trans) const {
            return bytes_[offsetSymbol_(trans)];
        }
        
        size_t getTransWords(size_t trans) const {
            size_t words = 0;
            auto size = isWordsLarge(trans) ? 1 + element_words_lower_size_ : 1;
            std::memcpy(&words, &bytes_[offsetUpperWords_(trans)], size);
            return words >> kNumFlags_;
        }
        
        size_t getGoto(size_t trans) const {
            assert(!isNextSet(trans));
            auto offset = offsetAddress_(trans);
            auto byte = bytes_[offset];
            size_t ret = static_cast<size_t>(byte & 0x7F);
            for (size_t shift = 7; byte & 0x80; shift += 7) {
                byte = bytes_[++offset];
                ret |= (byte & 0x7F) << shift;
            }
            return ret;
        }
        
        bool isFinalTrans(size_t trans) const {
            return static_cast<bool>(bytes_[offsetFlags_(trans)] & 1U);
        }
        
        bool isLastTrans(size_t trans) const {
            return static_cast<bool>(bytes_[offsetFlags_(trans)] & 2U);
        }
        
        bool isNextSet(size_t trans) const {
            return static_cast<bool>(bytes_[offsetFlags_(trans)] & 4U);
        }
        
        bool isWordsLarge(size_t trans) const {
            return static_cast<bool>(bytes_[offsetFlags_(trans)] & 8U);
        }
        
        // MARK: Functionals
        
        size_t getFirstTrans(size_t state) const {
            return state;
        }
        
        size_t skipTrans(size_t trans) const {
            return offsetAddress_(trans) + getAddrSize(trans);
        }
        
        size_t getNextTrans(size_t trans) const {
            return isLastTrans(trans) ? 0 : skipTrans(trans);
        }
        
        size_t getTargetState(size_t trans) const {
            if (isNextSet(trans)) {
                for (; !isLastTrans(trans); trans = getNextTrans(trans));
                return skipTrans(trans);
            } else {
                return getGoto(trans);
            }
        }
        
        size_t getRootState() const {
            return getTargetState(getFirstTrans(0));
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
            size += sizeof(element_words_lower_size_);
            size += size_vec(bytes_);
            return size;
        }
        
        void PrintForDebug(std::ostream& os) const {
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
            element_words_lower_size_ = read_val<size_t>(is);
            bytes_ = read_vec<uint8_t>(is);
        }
        
        void write(std::ostream &os) const {
            write_val(element_words_lower_size_, os);
            write_vec(bytes_, os);
        }
        
    private:
        const size_t kNumFlags_ = 4;
        const size_t kNodeSymbolSize_ = 1;
        const size_t kNodeWordsUpperBitsSize_ = 4;
        
        size_t element_words_lower_size_ = 4;
        std::vector<uint8_t> bytes_;
        
        // MARK: Offsets
        
        size_t offsetSymbol_(size_t trans) const {
            assert(trans < bytes_.size());
            return trans;
        }
        
        size_t offsetFlags_(size_t trans) const {
            assert(trans < bytes_.size());
            return offsetSymbol_(trans) + kNodeSymbolSize_;
        }
        
        size_t offsetUpperWords_(size_t trans) const {
            assert(trans < bytes_.size());
            return offsetFlags_(trans);
        }
        
        size_t offsetLowerWords(size_t trans) const {
            assert(trans < bytes_.size());
            return offsetUpperWords_(trans) + 1;
        }
        
        size_t offsetAddress_(size_t trans) const {
            assert(trans < bytes_.size());
            return offsetLowerWords(trans) + (isWordsLarge(trans) ? element_words_lower_size_ : 0);
        }
        
        size_t getAddrSize(size_t trans) const {
            assert(trans < bytes_.size());
            if (isNextSet(trans))
                return 0;
            auto offset = offsetAddress_(trans);
            auto size = 1;
            uint8_t byte = bytes_[offset];
            while (byte & 0x80) {
                byte = bytes_[offset + size++];
            }
            return size;
        }
        
    };
    
    
    MorfologikCFSA2DictionaryFoundation::MorfologikCFSA2DictionaryFoundation(const MorfologikCFSA2 &set) {
        struct Node {
            size_t words = 0;
            size_t offset = 0;
        };
        std::unordered_map<size_t, Node> nodes;
        
        const std::function<size_t(size_t)> dfs = [&set, &dfs, &nodes](size_t state) {
            size_t wordsCount = 0;
            for (auto t = set.get_first_trans(state); t != 0; t = set.get_next_trans(t)) {
                auto it = nodes.find(t);
                if (it != nodes.end()) {
                    wordsCount += it->second.words;
                } else {
                    size_t word = dfs(set.get_target_state(t));
                    if (set.is_final_trans(t))
                        word++;
                    nodes[t].words = word;
                    wordsCount += word;
                }
            }
            
            return wordsCount;
        };
        
        auto totalWords = dfs(set.get_root_state());
        element_words_lower_size_ = sim_ds::calc::SizeFitsInBytes(totalWords >> kNodeWordsUpperBitsSize_);
        
        for (size_t s = 0; s < set.bytes_.size(); s = set.skip_trans_(s)) {
            nodes[s].offset = std::numeric_limits<size_t>::max();
        }
        
        // Transpose fsa
        bool valueChanged = true;
        while (valueChanged) {
            valueChanged = false;
            
            bytes_.resize(0);
            for (size_t s = 0, offset = 0; s < set.bytes_.size(); s = set.skip_trans_(s)) {
                auto &node = nodes[s];
                
                assert(node.offset >= offset); // Every time target offset is decreasing repeats.
                if (node.offset != offset) {
                    valueChanged = true;
                    node.offset = offset;
                }
                
                // Enumerate parameters
                size_t flagsAndWords = set.is_final_trans(s) | (set.is_last_trans(s) << 1) | (set.is_next_set_(s) << 2);
                size_t words = node.words;
                flagsAndWords |= (words << kNumFlags_);
                auto sizePW = 1;
                bool isLargeWords = words >= (1ULL << kNodeWordsUpperBitsSize_);
                if (isLargeWords) {
                    flagsAndWords |= 0x08;
                    sizePW += element_words_lower_size_;
                }
                
                auto nodeSize = kNodeSymbolSize_ + sizePW;
                bytes_.resize(offset + nodeSize);
                uint8_t symbol = set.get_trans_symbol(s);
                // Transfer symbol
                bytes_[offset] = symbol;
                // Transfer flags
                std::memcpy(&bytes_[offset + kNodeSymbolSize_], &flagsAndWords, sizePW);
                
                // Transfer address
                if (!set.is_next_set_(s)) {
                    size_t value = nodes[set.get_target_state(s)].offset;
                    while (value > 0x7F) {
                        bytes_.emplace_back(0x80 | (value & 0x7F));
                        nodeSize++;
                        value >>= 7;
                    }
                    bytes_.emplace_back(static_cast<uint8_t>(value));
                    nodeSize++;
                }
                
                offset += nodeSize;
                
            }
            std::cerr << nodes[set.get_dest_state_offset_(0)].offset << std::endl;
        }
        
//        printForDebug(std::cout);
        
    }
    
}

#endif /* MorfologikCFSA2DictionaryFoundation_hpp */

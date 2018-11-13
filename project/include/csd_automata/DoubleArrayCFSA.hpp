
//  DoubleArrayCFSA.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/14.
//

#ifndef DoubleArrayCFSA_hpp
#define DoubleArrayCFSA_hpp

#include "IOInterface.hpp"

#include "DAFoundation.hpp"
#include "SerializedStrings.hpp"
#include "sim_ds/BitVector.hpp"
#include "ValueSet.hpp"

#include "CommonPrefixSet.hpp"

#include "DoubleArrayCFSABuilder.hpp"

namespace csd_automata {
    
    template<bool WORDS_CUMU, bool EDGE_LINK, bool ID_COMP, bool WORDS_COMP, bool NEEDS_ACCESS>
    class DoubleArrayCFSA : IOInterface {
    private:
        friend class DoubleArrayCFSABuilder;
        
    public:
        static std::string name() {
            return "DoubleArrayCFSA";
        }
        
        static constexpr bool useCumulativeWords = WORDS_CUMU;
        static constexpr bool useEdgeLink = EDGE_LINK;
        static constexpr bool shouldCompressID = ID_COMP;
        static constexpr bool shouldCompressWords = WORDS_COMP;
        static constexpr bool isPossibleAccess = NEEDS_ACCESS;
        
        static constexpr size_t kLookupError = 0;
        
        static constexpr bool useBinaryLabel = false;
        
        using foundation_type = DAFoundation<false, true, shouldCompressID, true, shouldCompressWords, useCumulativeWords, isPossibleAccess>;
        using strs_type = SerializedStrings<useBinaryLabel>;
        
        DoubleArrayCFSA() = default;
        
        explicit DoubleArrayCFSA(const DoubleArrayCFSABuilder& builder) {
            builder.release(*this);
        }
        
        explicit DoubleArrayCFSA(const DoubleArrayCFSABuilder& builder, ValueSet&& values) : DoubleArrayCFSA(builder) {
            values_ = std::move(values);
        }
        
        explicit DoubleArrayCFSA(const PlainFSA& fsa) {
            const auto shouldMergeSuffix = true;
            DoubleArrayCFSABuilder builder(fsa);
            builder.DoubleArrayCFSABuilder::build(useBinaryLabel, shouldMergeSuffix);
            builder.release(*this);
        }
        
        explicit DoubleArrayCFSA(const PlainFSA& fsa, ValueSet&& values) : DoubleArrayCFSA(fsa) {
            values_ = std::move(values);
        }
        
        explicit DoubleArrayCFSA(std::istream& is) {
            read(is);
        }
        
        void build(const DoubleArrayCFSABuilder& builder);
        
        bool isMember(const std::string& str) const;
        
        CommonPrefixSet commonPrefixSearch(const std::string& str) const;
        
        unsigned long long lookup(const std::string& str) const;
        
        std::string access(size_t key) const;
        
        bool hasBrother(size_t index) const {
            assert(EDGE_LINK);
            return has_brother_bits_[index];
        }
        
        uint8_t brother(size_t index) const {
            assert(EDGE_LINK);
            assert(has_brother_bits_[index]);
            return brother_[has_brother_bits_.rank(index)];
        }
        
        bool isNode(size_t index) const {
            assert(EDGE_LINK);
            return is_node_bits_[index];
        }
        
        uint8_t eldest(size_t index) const {
            assert(EDGE_LINK);
            assert(is_node_bits_[index]);
            return eldest_[is_node_bits_.rank(index)];
        }
        
        // MARK: - ByteData method
        
        size_t sizeInBytes() const override {
            auto size = sizeof(num_trans_);
            size += fd_.sizeInBytes();
            size += serialized_strings_.sizeInBytes();
            if constexpr (EDGE_LINK) {
                size += has_brother_bits_.sizeInBytes();
                size += size_vec(brother_);
                size += is_node_bits_.sizeInBytes();
                size += size_vec(eldest_);
            }
            size += values_.sizeInBytes();
            return size;
        }
        
        void write(std::ostream& os) const override {
            write_val(num_trans_, os);
            fd_.write(os);
            serialized_strings_.write(os);
            if constexpr (EDGE_LINK) {
                has_brother_bits_.write(os);
                write_vec(brother_, os);
                is_node_bits_.write(os);
                write_vec(eldest_, os);
            }
            values_.write(os);
        }
        
        void read(std::istream& is) override {
            num_trans_ = read_val<size_t>(is);
            fd_.read(is);
            serialized_strings_.read(is);
            if constexpr (EDGE_LINK) {
                has_brother_bits_.read(is);
                brother_ = read_vec<uint8_t>(is);
                is_node_bits_.read(is);
                eldest_ = read_vec<uint8_t>(is);
            }
            values_.read(is);
        }
        
        void showStatus(std::ostream& os) const override {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl
            << "#trans:\t" << num_trans_ << endl
            << "#elems:\t" << fd_.numElements() << endl
            << "size:\t" << sizeInBytes() << endl;
            fd_.showStatus(os);
            os << "\tstrings:\t" << serialized_strings_.sizeInBytes() << endl;
            if constexpr (EDGE_LINK) {
                os << "\tbrother:\t" << has_brother_bits_.sizeInBytes() + size_vec(brother_) << endl;
                os << "\teldest:\t" << is_node_bits_.sizeInBytes() + size_vec(eldest_) << endl;
            }
            os << "\tsize values:\t" << values_.sizeInBytes() << endl;
        }
        
        void printForDebug(std::ostream& os) const {
            using std::cout, std::endl;
            cout << "id\tT\tN\tC/S\tCW" << endl;
            for (auto i = 0; i < num_trans_; i++) {
                cout << i << '\t' << fd_.isFinal(i) << '\t' << fd_.next(i) << '\t';
                if (!fd_.isString(i)) {
                    cout << fd_.check(i);
                } else {
                    cout << serialized_strings_.string_view(fd_.stringId(i));
                }
                cout << '\t' << fd_.words(i) << endl;
            }
        }
        
        // MARK: Copy guard
        
        ~DoubleArrayCFSA() = default;
        
        DoubleArrayCFSA (const DoubleArrayCFSA&) = delete;
        DoubleArrayCFSA& operator=(const DoubleArrayCFSA&) = delete;
        
        DoubleArrayCFSA(DoubleArrayCFSA&& rhs) noexcept = default;
        DoubleArrayCFSA& operator=(DoubleArrayCFSA&& rhs) noexcept = default;
        
    private:
        size_t num_trans_ = 0;
        foundation_type fd_;
        strs_type serialized_strings_;
        // If use EDGE_LINK
        sim_ds::BitVector has_brother_bits_;
        std::vector<uint8_t> brother_;
        sim_ds::BitVector is_node_bits_;
        std::vector<uint8_t> eldest_;
        
        // If set values
        ValueSet values_;
        
        // MARK: Getter
        
        size_t target(size_t index) const {
            return fd_.next(index) ^ index;
        }
        
        size_t transition(size_t prevTrans, uint8_t label) const {
            return target(prevTrans) ^ label;
        }
        
        // MARK: Protocol setting
        
        void resize(size_t num, size_t words) {
            fd_.resize(num, words);
            if constexpr (EDGE_LINK) {
                has_brother_bits_.resize(num);
                is_node_bits_.resize(num);
            }
        }
        
        void setNumTrans(size_t num) {
            num_trans_ = num;
        }
        
        // MARK: For build
        
        void setCheck(size_t index, uint8_t check) {
            fd_.setCheck(index, check);
        }
        
        void setNext(size_t index, size_t next) {
            fd_.setNext(index, next);
        }
        
        void setIsStringTrans(size_t index, bool isString) {
            fd_.setIsString(index, isString);
        }
        
        void setIsFinal(size_t index, size_t isFinal) {
            fd_.setIsFinal(index, isFinal);
        }
        
        void setStringIndex(size_t index, size_t strIndex) {
            fd_.setStringId(index, strIndex);
        }
        
        void setStringArray(strs_type &&sArr) {
            serialized_strings_ = std::move(sArr);
        }
        
        void setWords(size_t index, size_t store) {
            fd_.setWords(index, store);
        }
        
        void setCumWords(size_t index, size_t as) {
            fd_.setCumWords(index, as);
        }
        
        void setHasBrother(size_t index, bool has) {
            has_brother_bits_[index] = has;
        }
        
        void setBrother(size_t index, uint8_t bro) {
            brother_.emplace_back(bro);
        }
        
        void setIsNode(size_t index, bool isNode) {
            is_node_bits_[index] = isNode;
        }
        
        void setEldest(size_t index, uint8_t eldest) {
            eldest_.emplace_back(eldest);
        }
        
        void buildBitVector() {
            fd_.build();
            if constexpr (EDGE_LINK) {
                has_brother_bits_.build();
                is_node_bits_.build();
            }
        }
        
    };
    
    
    template<bool C, bool E, bool I, bool W, bool NA>
    bool DoubleArrayCFSA<C, E, I, W, NA>::isMember(const std::string& str) const {
        size_t trans = 0;
        for (size_t pos = 0, size = str.size(); pos < size; pos++) {
            uint8_t c = str[pos];
            trans = transition(trans, c);
            if (!fd_.isString(trans)) {
                // Check label that is character
                uint8_t checkE = fd_.check(trans);
                if (checkE != c)
                    return false;
            } else {
                // Check label that is indexed string
                auto sid = fd_.stringId(trans);
                if (!serialized_strings_.match(&pos, str, sid)) {
#ifndef NDEBUG
                    serialized_strings_.showLabels(sid - 32, sid + 32);
#endif
                    return false;
                }
            }
        }
        return fd_.isFinal(trans);
    }
    
    
    template<bool C, bool E, bool I, bool W, bool NA>
    CommonPrefixSet DoubleArrayCFSA<C, E, I, W, NA>::commonPrefixSearch(const std::string& str) const {
        CommonPrefixSet prefixSet(str);
        size_t counter = 0;
        
        size_t trans = 0;
        for (size_t pos = 0, size = str.size(); pos < size; pos++) {
            uint8_t c = str[pos];
            trans = transition(trans, c);
            if (!fd_.isString(trans)) {
                auto checkE = fd_.check(trans);
                if (checkE != c)
                    break;
            } else {
                auto sid = fd_.stringId(trans);
                if (!serialized_strings_.match(&pos, str, sid)) {
#ifndef NDEBUG
                    serialized_strings_.showLabels(sid - 32, sid + 32);
#endif
                    break;
                }
            }
            counter += fd_.cumWords(trans);
            if (fd_.isFinal(trans)) {
                counter++;
                prefixSet.appendPrefix(pos + 1, counter);
            }
        }
        return prefixSet;
    }
    
    
    template<bool C, bool E, bool I, bool W, bool NA>
    unsigned long long DoubleArrayCFSA<C, E, I, W, NA>::lookup(const std::string& str) const {
        size_t trans = 0;
        size_t counter = 0;
        for (size_t pos = 0, size = str.size(); pos < size; pos++) {
            if (trans > 0 && fd_.isFinal(trans))
                counter++;
            
            uint8_t c = str[pos];
            
            // Separate algorithm from template parameters, that is to use cumulative-words.
            if constexpr (C) {
                trans = transition(trans, c);
                
                if (!fd_.isString(trans)) {
                    const uint8_t checkE = fd_.check(trans);
                    if (checkE != c)
                        return kLookupError;
                } else {
                    const auto sid = fd_.stringId(trans);
                    if (!serialized_strings_.match(&pos, str, sid)) { // Increment 'pos'
#ifndef NDEBUG
                        serialized_strings_.showLabels(sid - 32, sid + 32);
#endif
                        return kLookupError;
                    }
                }
                
                counter += fd_.cumWords(trans);
            } else {
                size_t nextTrans = -1;
                for (size_t label = 1; label <= 0xFF; label++) {
                    size_t nt = transition(trans, label);
                    size_t checkType;
                    
                    bool isStr = fd_.isString(nt);
                    if (!isStr) {
                        checkType = fd_.check(nt);
                        if (checkType != label)
                            continue;
                    } else {
                        checkType = fd_.stringId(nt);
                        if (serialized_strings_[checkType] != label)
                            continue;
                    }
                    if (c == label) {
                        if (isStr && !serialized_strings_.match(&pos, str, checkType)) { // Increment 'pos'
#ifndef NDEBUG
                            serialized_strings_.showLabels(checkType - 32, checkType + 32);
#endif
                            return kLookupError;
                        }
                        
                        nextTrans = nt;
                        break;
                    }
                    
                    counter += fd_.words(nt);
                }
                if (nextTrans == -1)
                    return kLookupError;
                trans = nextTrans;
            }
        }
        
        return fd_.isFinal(trans) ? ++counter : kLookupError;
    }
    
    
    template<bool C, bool E, bool I, bool W, bool NA>
    std::string DoubleArrayCFSA<C, E, I, W, NA>::access(size_t key) const {
        assert(NA);
        
        size_t trans = 0;
        size_t counter = key;
        std::string str = "";
        while (counter > 0) {
            auto targetNode = target(trans);
            size_t nextTrans = kLookupError;
            uint8_t c = (E ? eldest(targetNode) : 1);
            while (true) {
                size_t nt = targetNode ^ c;
                size_t checkType;
                
                bool isStr = fd_.isString(nt);
                bool checkClear;
                if (!isStr) {
                    checkType = fd_.check(nt);
                    checkClear = checkType == c;
                } else {
                    checkType = fd_.stringId(nt);
                    checkClear = uint8_t(serialized_strings_[checkType]) == c;
                }
                if (!checkClear) {
                    if constexpr (E) {
                        return "";
                    } else {
                        c++;
                        continue;
                    }
                }
                
                auto curStore = fd_.words(nt);
                if (curStore < counter) {
                    counter -= curStore;
                    if constexpr (E) {
                        if (!hasBrother(nt))
                            return "";
                        c = brother(nt);
                    } else {
                        if (c == 0xff)
                            break;
                        c++;
                    }
                } else {
                    if (fd_.isFinal(nt))
                        counter--;
                    if (!isStr)
                        str += checkType;
                    else
                        str += serialized_strings_.string_view(checkType);
                    
                    nextTrans = nt;
                    break;
                }
            }
            
            if (nextTrans == kLookupError)
                return "";
            
            trans = nextTrans;
        }
        
        return str;
        
    }
    
}

#endif /* DoubleArrayCFSA_hpp */

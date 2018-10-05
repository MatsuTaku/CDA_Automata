
//  DoubleArrayCFSA.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/14.
//

#ifndef DoubleArrayCFSA_hpp
#define DoubleArrayCFSA_hpp

#include "ByteData.hpp"

#include "DAFoundation.hpp"
#include "StringArray.hpp"
#include "sim_ds/BitVector.hpp"

#include "DoubleArrayCFSABuilder.hpp"

namespace csd_automata {
    
    class PlainFSA;
    
    template<bool WORDS_CUMU, bool EDGE_LINK, bool ID_COMP, bool WORDS_COMP, bool NEEDS_ACCESS>
    class DoubleArrayCFSA : ByteData {
        friend class DoubleArrayCFSABuilder;
    public:
        static constexpr bool useCumulativeWords = WORDS_CUMU;
        static constexpr bool useEdgeLink = EDGE_LINK;
        static constexpr bool shouldCompressID= ID_COMP;
        static constexpr bool shouldCompressWords = WORDS_COMP;
        static constexpr bool isPossibleAccess = NEEDS_ACCESS;
        
        static constexpr bool useBinaryLabel = false;
        static constexpr size_t searchError = -1;
        
        using foundation_type = DAFoundation<false, true, shouldCompressID, true, shouldCompressWords, useCumulativeWords, isPossibleAccess>;
        using strs_type = StringArray<useBinaryLabel>;

        DoubleArrayCFSA() = default;
        
        explicit DoubleArrayCFSA(const DoubleArrayCFSABuilder &builder) {
            builder.release(*this);
        }
        
        explicit DoubleArrayCFSA(const PlainFSA &fsa) {
            const auto shouldMergeSuffix = true;
            DoubleArrayCFSABuilder builder(fsa);
            builder.DoubleArrayCFSABuilder::build(useBinaryLabel, shouldMergeSuffix);
            builder.release(*this);
        }
        
        explicit DoubleArrayCFSA(std::istream &is) {
            read(is);
        }
        
        ~DoubleArrayCFSA() = default;
        
        // MARK: - Copy guard
        
        DoubleArrayCFSA (const DoubleArrayCFSA&) = delete;
        DoubleArrayCFSA& operator=(const DoubleArrayCFSA&) = delete;
        
        DoubleArrayCFSA(DoubleArrayCFSA&& rhs) noexcept = default;
        DoubleArrayCFSA& operator=(DoubleArrayCFSA&& rhs) noexcept = default;
        
    public:
        static std::string name() {
            return "DoubleArrayCFSA";
        }
        
        void build(const DoubleArrayCFSABuilder &builder);
        
        bool isMember(const std::string &str) const;
        
        unsigned long long lookup(const std::string &str) const;
        
        std::string access(size_t key) const;
        
        size_t target(size_t index) const {
            return next(index) ^ index;
        }
        
        size_t next(size_t index) const {
            return fd_.next(index);
        }
        
        uint8_t check(size_t index) const {
            return fd_.check(index);
        }
        
        size_t stringId(size_t index) const {
            assert(isStringTrans(index));
            return fd_.stringId(index);
        }
        
        bool isFinal(size_t index) const {
            return fd_.isFinal(index);
        }
        
        bool isStringTrans(size_t index) const {
            return fd_.isString(index);
        }
        
        size_t store(size_t index) const {
            return fd_.words(index);
        }
        
        size_t accStore(size_t index) const {
            return fd_.cumWords(index);
        }
        
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
            //            size += is_string_bits_.sizeInBytes();
            size += strings_.sizeInBytes();
            if constexpr (EDGE_LINK) {
                size += has_brother_bits_.sizeInBytes();
                size += size_vec(brother_);
                size += is_node_bits_.sizeInBytes();
                size += size_vec(eldest_);
            }
            return size;
        }
        
        void write(std::ostream& os) const override {
            write_val(num_trans_, os);
            fd_.write(os);
            strings_.write(os);
            if constexpr (EDGE_LINK) {
                has_brother_bits_.write(os);
                write_vec(brother_, os);
                is_node_bits_.write(os);
                write_vec(eldest_, os);
            }
        }
        
        void writeCheck(std::ostream &os) const {
            std::vector<size_t> checks(fd_.numElements());
            for (auto i = 0; i < fd_.numElements(); i++) {
                checks[i] = isStringTrans(i) ? fd_.stringId(i) : fd_.check(i);
            }
            sim_ds::Vector vec(checks);
            vec.write(os);
        }
        
        void read(std::istream& is) override {
            num_trans_ = read_val<size_t>(is);
            fd_.read(is);
            strings_.read(is);
            if constexpr (EDGE_LINK) {
                has_brother_bits_.read(is);
                brother_ = read_vec<uint8_t>(is);
                is_node_bits_.read(is);
                eldest_ = read_vec<uint8_t>(is);
            }
        }
        
        void showStatus(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << num_trans_ << endl;
            os << "#elems: " << fd_.numElements() << endl;
            os << "size:   " << sizeInBytes() << endl;
            fd_.showStatus(os);
            os << "size strings:    " << strings_.sizeInBytes() << endl;
            if constexpr (EDGE_LINK) {
                os << "size brother:    " << has_brother_bits_.sizeInBytes() + size_vec(brother_) << endl;
                os << "size eldest:    " << is_node_bits_.sizeInBytes() + size_vec(eldest_) << endl;
            }
        }
        
        void printForDebug(std::ostream& os) const {
            using std::cout, std::endl;
            cout << "id\tT\tN\tC/S\tCW" << endl;
            for (auto i = 0; i < num_trans_; i++) {
                cout << i << '\t' << fd_.isFinal(i) << '\t' << fd_.next(i) << '\t';
                if (!fd_.isString(i)) {
                    cout << fd_.check(i);
                } else {
                    cout << strings_.string_view(fd_.stringId(i));
                }
                cout << '\t' << fd_.words(i) << endl;
            }
        }
        
    private:
        size_t num_trans_ = 0;
        foundation_type fd_;
        strs_type strings_;
        // If use EDGE_LINK
        sim_ds::BitVector has_brother_bits_;
        std::vector<uint8_t> brother_;
        sim_ds::BitVector is_node_bits_;
        std::vector<uint8_t> eldest_;
        
        // MARK: - Protocol setting
        
        void resize(size_t num, size_t words) {
            fd_.resize(num, words);
            has_brother_bits_.resize(num);
            is_node_bits_.resize(num);
        }
        
        void setNumTrans(size_t num) {
            num_trans_ = num;
        }
        
        // MARK: - build
        
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
            strings_ = std::move(sArr);
        }
        
        void setWords(size_t index, size_t store) {
            fd_.setWords(index, store);
        }
        
        void setCumWords(size_t index, size_t as) {
            fd_.setCumWords(index, as);
        }
        
        void setHasBrother(size_t index, bool has) {
            has_brother_bits_.set(index, has);
        }
        
        void setBrother(size_t index, uint8_t bro) {
            brother_.emplace_back(bro);
        }
        
        void setIsNode(size_t index, bool isNode) {
            is_node_bits_.set(index, isNode);
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
    
    
//    template<bool C, bool E, bool I, bool W, bool NA>
//    void DoubleArrayCFSA<C, E, I, W, NA>::build(const DoubleArrayCFSABuilder& builder) {
//        const auto numElems = builder.numElems_();
//        resize(numElems, builder.getNumWords());
//        auto sab = stringArrayBuilder(builder);
//        setStringArray(sa_type(sab));
//
//        auto numTrans = 0;
//        for (auto i = 0; i < numElems; i++) {
//            if (builder.isFrozen_(i)) {
//                numTrans++;
//
//                auto isStrTrans = builder.hasLabel_(i);
//                setNext(i, builder.getNext_(i));
//                setIsStringTrans(i, isStrTrans);
//                setIsFinal(i, builder.isFinal_(i));
//                setCheck(i, builder.getCheck_(i));
//                if (isStrTrans)
//                    setStringIndex(i, builder.getLabelNumber_(i));
//                else
//                    setCheck(i, builder.getCheck_(i));
//
//                if constexpr (NA)
//                    setWords(i, builder.getStore_(i));
//                if constexpr (C)
//                    setCumWords(i, builder.getAccStore_(i));
//
//                if constexpr (E) {
//                    bool hasBro = builder.hasBrother_(i);
//                    setHasBrother(i, hasBro);
//                    if (hasBro)
//                        setBrother(i, builder.getBrother_(i));
//                }
//            }
//
//            if constexpr (E) {
//                bool isNode = builder.isUsedNext_(i);
//                setIsNode(i, isNode);
//                if (isNode)
//                    setEldest(i, builder.getEldest_(i));
//            }
//        }
//        buildBitVector();
//        setNumTrans(numTrans);
//
//        builder.showCompareWith(*this);
//
//    }
    
    template<bool C, bool E, bool I, bool W, bool NA>
    bool DoubleArrayCFSA<C, E, I, W, NA>::isMember(const std::string &str) const {
        size_t trans = 0;
        for (size_t pos = 0, size = str.size(); pos < size; pos++) {
            uint8_t c = str[pos];
            trans = target(trans) ^ c;
            if (!isStringTrans(trans)) {
                auto checkE = check(trans);
                if (checkE != c)
                    return false;
            } else {
                auto sid = stringId(trans);
                if (!strings_.isMatch(&pos, str, sid)) {
                    strings_.showLabels(sid - 32, sid + 32);
                    return false;
                }
            }
        }
        return isFinal(trans);
    }
    
    template<bool C, bool E, bool I, bool W, bool NA>
    unsigned long long DoubleArrayCFSA<C, E, I, W, NA>::lookup(const std::string &str) const {
        size_t trans = 0;
        size_t counter = 0;
        for (size_t pos = 0, size = str.size(); pos < size; pos++) {
            if (trans > 0 && isFinal(trans))
                counter++;
            
            uint8_t c = str[pos];
            
            // Separate algorithm from template parameters, that is to use cumulative-words.
            if constexpr (C) {
                trans = target(trans) ^ c;
                if (!isStringTrans(trans)) {
                    auto checkE = check(trans);
                    if (checkE != c)
                        return searchError;
                } else {
                    auto sid = stringId(trans);
                    if (!strings_.isMatch(&pos, str, sid)) {
                        strings_.showLabels(sid - 32, sid + 32);
                        return searchError;
                    }
                }
                
                counter += accStore(trans);
                
            } else {
                size_t nextTrans = searchError;
                for (size_t label = 1; label <= 0xFF; label++) {
                    size_t nt = target(trans) ^ label;
                    size_t checkType;
                    
                    bool isStr = isStringTrans(nt);
                    if (!isStr) {
                        checkType = check(nt);
                        if (checkType != label)
                            continue;
                    } else {
                        checkType = stringId(nt);
                        if (strings_[checkType] != label)
                            continue;
                    }
                    if (c == label) {
                        if (isStr && !strings_.isMatch(&pos, str, checkType)) { // to increment pos in source string
                            strings_.showLabels(checkType - 32, checkType + 32);
                            return searchError;
                        }
                        
                        nextTrans = nt;
                        break;
                    }
                    
                    counter += store(nt);
                }
                
                if (nextTrans == searchError)
                    return searchError;
                
                trans = nextTrans;
                
            }
        }
        return isFinal(trans) ? ++counter : searchError;
    }
    
    template<bool C, bool E, bool I, bool W, bool NA>
    std::string DoubleArrayCFSA<C, E, I, W, NA>::access(size_t key) const {
        size_t trans = 0;
        size_t counter = key;
        std::string str = "";
        while (counter > 0) {
            auto targetNode = target(trans);
            size_t nextTrans = searchError;
            uint8_t c = E ? eldest(targetNode) : 1;
            while (true) {
                size_t nt = targetNode ^ c;
                size_t checkType;
                
                bool isStr = isStringTrans(nt);
                bool checkClear;
                if (!isStr) {
                    checkType = check(nt);
                    checkClear = checkType == c;
                } else {
                    checkType = stringId(nt);
                    checkClear = strings_[checkType] == c;
                }
                if (!checkClear) {
                    if constexpr (E) {
                        return "";
                    } else {
                        c++;
                        continue;
                    }
                }
                
                auto curStore = store(nt);
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
                    if (isFinal(nt))
                        counter--;
                    if (!isStr)
                        str += checkType;
                    else
                        str += strings_.string_view(checkType);
                    
                    nextTrans = nt;
                    break;
                }
            }
            
            if (nextTrans == searchError)
                return "";
            
            trans = nextTrans;
        }
        
        return str;
        
    }
    
}

#endif /* DoubleArrayCFSA_hpp */

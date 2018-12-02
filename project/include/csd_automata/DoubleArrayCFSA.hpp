
//  DoubleArrayCFSA.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/14.
//

#ifndef DoubleArrayCFSA_hpp
#define DoubleArrayCFSA_hpp

#include "IOInterface.hpp"

#include "DAFoundation.hpp"
#include "sim_ds/BitVector.hpp"
#include "SerializedStrings.hpp"
#include "ValueSet.hpp"

#include "CommonPrefixSet.hpp"

#include "DoubleArrayCFSABuilder.hpp"

namespace csd_automata {
    
template<bool UseCumulativeWords, bool LinkChildren, bool CompressStrId, bool CompressWords, bool SupportAccess>
class DoubleArrayCFSA : IOInterface {
    
    static constexpr bool kUseCumulativeWords = UseCumulativeWords;
    static constexpr bool kLinkChildren = LinkChildren;
    static constexpr bool kCompressStrID = CompressStrId;
    static constexpr bool kCompressWords = CompressWords;
    static constexpr bool kSupportAccess = SupportAccess;
    
    static constexpr size_t kLookupError = 0;
    
    static constexpr bool kCompressNext = false;
    static constexpr bool kUseStrId = true;
    static constexpr bool kHashing = true;
    using Foundation = DAFoundation<kCompressNext, kUseStrId, kCompressStrID, kHashing, kCompressWords, kUseCumulativeWords, kSupportAccess>;
    
    static constexpr bool kMergeSuffixOfSerializedStrings = true;
    static constexpr bool kUseBinaryLabel = false;
    using StringsStorage = SerializedStrings<kUseBinaryLabel>;
    
    using BitVector = sim_ds::BitVector;
    
    size_t num_trans_ = 0;
    Foundation base_;
    StringsStorage serialized_strings_;
    // If use EDGE_LINK
    BitVector has_brother_bits_;
    std::vector<uint8_t> brother_;
    BitVector is_node_bits_;
    std::vector<uint8_t> eldest_;
    
    // If set values
    ValueSet values_;
    
    friend class DoubleArrayCFSABuilder;
    
    using Builder = DoubleArrayCFSABuilder;
    
public:
    
    static std::string name() {
        return "DoubleArrayCFSA";
    }
    
    explicit DoubleArrayCFSA(const Builder& builder) {
        builder.release(*this);
    }
    
    explicit DoubleArrayCFSA(const Builder& builder, ValueSet&& values) : DoubleArrayCFSA(builder) {
        values_ = std::move(values);
    }
    
    explicit DoubleArrayCFSA(const PlainFSA& fsa) {
        Builder builder(fsa);
        builder.build(kUseBinaryLabel, kMergeSuffixOfSerializedStrings);
        builder.release(*this);
    }
    
    explicit DoubleArrayCFSA(const PlainFSA& fsa, ValueSet&& values) : DoubleArrayCFSA(fsa) {
        values_ = std::move(values);
    }
    
    explicit DoubleArrayCFSA(std::istream& is) {
        Read(is);
    }
    
    bool Accept(const std::string& str) const;
    
    CommonPrefixSet CommonPrefixSearch(const std::string& str) const;
    
    unsigned long long Lookup(const std::string& str) const;
    
    std::string Access(size_t key) const;
    
    bool has_brother(size_t index) const {
        assert(kLinkChildren);
        return has_brother_bits_[index];
    }
    
    uint8_t brother(size_t index) const {
        assert(kLinkChildren);
        assert(has_brother_bits_[index]);
        return brother_[has_brother_bits_.rank(index)];
    }
    
    bool is_node(size_t index) const {
        assert(kLinkChildren);
        return is_node_bits_[index];
    }
    
    uint8_t eldest(size_t index) const {
        assert(kLinkChildren);
        assert(is_node_bits_[index]);
        return eldest_[is_node_bits_.rank(index)];
    }
    
    // MARK: - ByteData method
    
    size_t size_in_bytes() const override {
        auto size = sizeof(num_trans_);
        size += base_.size_in_bytes();
        size += serialized_strings_.size_in_bytes();
        if constexpr (kLinkChildren) {
            size += has_brother_bits_.size_in_bytes();
            size += size_vec(brother_);
            size += is_node_bits_.size_in_bytes();
            size += size_vec(eldest_);
        }
        size += values_.size_in_bytes();
        return size;
    }
    
    void Read(std::istream& is) override {
        num_trans_ = read_val<size_t>(is);
        base_.Read(is);
        serialized_strings_.Read(is);
        if constexpr (kLinkChildren) {
            has_brother_bits_.Read(is);
            brother_ = read_vec<uint8_t>(is);
            is_node_bits_.Read(is);
            eldest_ = read_vec<uint8_t>(is);
        }
        values_.Read(is);
    }
    
    void Write(std::ostream& os) const override {
        write_val(num_trans_, os);
        base_.Write(os);
        serialized_strings_.Write(os);
        if constexpr (kLinkChildren) {
            has_brother_bits_.Write(os);
            write_vec(brother_, os);
            is_node_bits_.Write(os);
            write_vec(eldest_, os);
        }
        values_.Write(os);
    }
    
    void ShowStats(std::ostream& os) const override {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl
        << "#trans:\t" << num_trans_ << endl
        << "#elems:\t" << base_.num_elements() << endl
        << "size:\t" << size_in_bytes() << endl;
        base_.ShowStats(os);
        os << "\tstrings:\t" << serialized_strings_.size_in_bytes() << endl;
        if constexpr (kLinkChildren) {
            os << "\tbrother:\t" << has_brother_bits_.size_in_bytes() + size_vec(brother_) << endl;
            os << "\teldest:\t" << is_node_bits_.size_in_bytes() + size_vec(eldest_) << endl;
        }
        os << "\tsize values:\t" << values_.size_in_bytes() << endl;
    }
    
    void PrintForDebug(std::ostream& os) const {
        using std::cout, std::endl;
        cout << "id\tT\tN\tC/S\tCW" << endl;
        for (auto i = 0; i < num_trans_; i++) {
            cout << i << '\t' << base_.is_final(i) << '\t' << base_.next(i) << '\t';
            if (!base_.is_string(i)) {
                cout << base_.check(i);
            } else {
                cout << serialized_strings_.string_view(base_.string_id(i));
            }
            cout << '\t' << base_.words(i) << endl;
        }
    }
    
    // MARK: Copy guard
    
    DoubleArrayCFSA() = default;
    ~DoubleArrayCFSA() = default;
    
    DoubleArrayCFSA (const DoubleArrayCFSA&) = delete;
    DoubleArrayCFSA& operator=(const DoubleArrayCFSA&) = delete;
    
    DoubleArrayCFSA(DoubleArrayCFSA&& rhs) noexcept = default;
    DoubleArrayCFSA& operator=(DoubleArrayCFSA&& rhs) noexcept = default;
    
private:
    
    // MARK: Getter
    
    size_t target_(size_t index) const {
        return base_.next(index) ^ index;
    }
    
    size_t transition_(size_t prevTrans, uint8_t label) const {
        return target_(prevTrans) ^ label;
    }
    
    // MARK: Protocol setting
    
    void resize(size_t num, size_t words) {
        base_.resize(num, words);
        if constexpr (kLinkChildren) {
            has_brother_bits_.resize(num);
            is_node_bits_.resize(num);
        }
    }
    
    void set_num_trans(size_t num) {
        num_trans_ = num;
    }
    
    // MARK: For build
    
    void set_check(size_t index, uint8_t check) {
        base_.set_check(index, check);
    }
    
    void set_next(size_t index, size_t next) {
        base_.set_next(index, next);
    }
    
    void set_is_string_trans(size_t index, bool isString) {
        base_.set_is_string(index, isString);
    }
    
    void set_is_final(size_t index, size_t isFinal) {
        base_.set_is_final(index, isFinal);
    }
    
    void set_string_id(size_t index, size_t strIndex) {
        base_.set_string_id(index, strIndex);
    }
    
    void set_serialized_strings(StringsStorage &&sArr) {
        serialized_strings_ = std::move(sArr);
    }
    
    void set_words(size_t index, size_t store) {
        base_.set_words(index, store);
    }
    
    void set_cum_words(size_t index, size_t as) {
        base_.set_cum_words(index, as);
    }
    
    void set_has_brother(size_t index, bool has) {
        has_brother_bits_[index] = has;
    }
    
    void set_brother(size_t index, uint8_t bro) {
        brother_.emplace_back(bro);
    }
    
    void set_is_node(size_t index, bool isNode) {
        is_node_bits_[index] = isNode;
    }
    
    void set_eldest(size_t index, uint8_t eldest) {
        eldest_.emplace_back(eldest);
    }
    
    void BuildBitVector() {
        base_.Build();
        if constexpr (kLinkChildren) {
            has_brother_bits_.Build();
            is_node_bits_.Build();
        }
    }
    
};


template<bool C, bool E, bool I, bool W, bool NA>
bool DoubleArrayCFSA<C, E, I, W, NA>::Accept(const std::string& str) const {
    size_t trans = 0;
    for (size_t pos = 0, size = str.size(); pos < size; pos++) {
        uint8_t c = str[pos];
        trans = transition_(trans, c);
        if (!base_.is_string(trans)) {
            // Check label that is character
            uint8_t checkE = base_.check(trans);
            if (checkE != c)
                return false;
        } else {
            // Check label that is indexed string
            auto sid = base_.string_id(trans);
            if (!serialized_strings_.match(&pos, str, sid)) {
#ifndef NDEBUG
                serialized_strings_.ShowLabels(sid - 32, sid + 32);
#endif
                return false;
            }
        }
    }
    return base_.is_final(trans);
}


template<bool C, bool E, bool I, bool W, bool NA>
CommonPrefixSet DoubleArrayCFSA<C, E, I, W, NA>::CommonPrefixSearch(const std::string& str) const {
    CommonPrefixSet prefixSet(str);
    size_t counter = 0;
    
    size_t trans = 0;
    for (size_t pos = 0, size = str.size(); pos < size; pos++) {
        uint8_t c = str[pos];
        trans = transition_(trans, c);
        if (!base_.is_string(trans)) {
            auto checkE = base_.check(trans);
            if (checkE != c)
                break;
        } else {
            auto sid = base_.string_id(trans);
            if (!serialized_strings_.match(&pos, str, sid)) {
#ifndef NDEBUG
                serialized_strings_.ShowLabels(sid - 32, sid + 32);
#endif
                break;
            }
        }
        counter += base_.cum_words(trans);
        if (base_.is_final(trans)) {
            counter++;
            prefixSet.AppendPrefix(pos + 1, counter);
        }
    }
    return prefixSet;
}


template<bool C, bool E, bool I, bool W, bool NA>
unsigned long long DoubleArrayCFSA<C, E, I, W, NA>::Lookup(const std::string& str) const {
    size_t trans = 0;
    size_t counter = 0;
    for (size_t pos = 0, size = str.size(); pos < size; pos++) {
        if (trans > 0 && base_.is_final(trans))
            counter++;
        
        uint8_t c = str[pos];
        
        // Separate algorithm from template parameters, that is to use cumulative-words.
        if constexpr (C) {
            trans = transition_(trans, c);
            
            if (!base_.is_string(trans)) {
                const uint8_t checkE = base_.check(trans);
                if (checkE != c)
                    return kLookupError;
            } else {
                const auto sid = base_.string_id(trans);
                if (!serialized_strings_.match(&pos, str, sid)) { // Increment 'pos'
#ifndef NDEBUG
                    serialized_strings_.ShowLabels(sid - 32, sid + 32);
#endif
                    return kLookupError;
                }
            }
            
            counter += base_.cum_words(trans);
        } else {
            size_t nextTrans = -1;
            for (size_t label = 1; label <= 0xFF; label++) {
                size_t nt = transition_(trans, label);
                size_t checkType;
                
                bool isStr = base_.is_string(nt);
                if (!isStr) {
                    checkType = base_.check(nt);
                    if (checkType != label)
                        continue;
                } else {
                    checkType = base_.string_id(nt);
                    if (serialized_strings_[checkType] != label)
                        continue;
                }
                if (c == label) {
                    if (isStr && !serialized_strings_.match(&pos, str, checkType)) { // Increment 'pos'
#ifndef NDEBUG
                        serialized_strings_.ShowLabels(checkType - 32, checkType + 32);
#endif
                        return kLookupError;
                    }
                    
                    nextTrans = nt;
                    break;
                }
                
                counter += base_.words(nt);
            }
            if (nextTrans == -1)
                return kLookupError;
            trans = nextTrans;
        }
    }
    
    return base_.is_final(trans) ? ++counter : kLookupError;
}


template<bool C, bool E, bool I, bool W, bool NA>
std::string DoubleArrayCFSA<C, E, I, W, NA>::Access(size_t key) const {
    assert(NA);
    
    size_t trans = 0;
    size_t counter = key;
    std::string str = "";
    while (counter > 0) {
        auto targetNode = target_(trans);
        size_t nextTrans = kLookupError;
        uint8_t c = (E ? eldest(targetNode) : 1);
        while (true) {
            size_t nt = targetNode ^ c;
            size_t checkType;
            
            bool isStr = base_.is_string(nt);
            bool checkClear;
            if (!isStr) {
                checkType = base_.check(nt);
                checkClear = checkType == c;
            } else {
                checkType = base_.string_id(nt);
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
            
            auto curStore = base_.words(nt);
            if (curStore < counter) {
                counter -= curStore;
                if constexpr (E) {
                    if (!has_brother(nt))
                        return "";
                    c = brother(nt);
                } else {
                    if (c == 0xff)
                        break;
                    c++;
                }
            } else {
                if (base_.is_final(nt))
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

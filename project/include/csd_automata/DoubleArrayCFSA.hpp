
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
    
class AutomataExplorer {
public:
    AutomataExplorer(std::string_view text) : trans_(0), text_(text), pos_on_text_(0) {}
    
    void set_trans(size_t new_value) {
        trans_ = new_value;
    }
    
    size_t trans() const {
        return trans_;
    }
    
    std::string_view text() const {
        return text_;
    }
    
    void set_pos(size_t new_value) {
        pos_on_text_ = new_value;
    }
    
    size_t pos() const {
        return pos_on_text_;
    }
    
    template <typename T>
    void observe(T new_value) {
        observed_ = new_value;
    }
    
    template <typename T = id_type>
    T observed() const {
        return T(observed_);
    }
    
private:
    size_t trans_;
    std::string_view text_;
    size_t pos_on_text_;
    id_type observed_ = 0;
};


template<bool UnionCheckAndId, bool UseCumulativeWords, bool LinkChildren, bool CompressStrId, bool CompressWords, bool SupportAccess>
class DoubleArrayCFSA : IOInterface {
public:
    using Self = DoubleArrayCFSA<UnionCheckAndId, UseCumulativeWords, LinkChildren, CompressStrId, CompressWords, SupportAccess>;
    
    static constexpr bool kUnionCheckAndId = UnionCheckAndId;
    static constexpr bool kUseCumulativeWords = UseCumulativeWords;
    static constexpr bool kLinkChildren = LinkChildren;
    static constexpr bool kCompressStrID = CompressStrId;
    static constexpr bool kCompressWords = CompressWords;
    static constexpr bool kSupportAccess = SupportAccess;
    
    static constexpr size_t kSearchError = 0;
    
    static constexpr bool kCompressNext = false;
    static constexpr bool kUseStrId = true;
    static constexpr bool kHashing = true;
    using Foundation = DAFoundation<kCompressNext, kUseStrId, kUnionCheckAndId, kCompressStrID, kHashing, kCompressWords, kUseCumulativeWords, kSupportAccess>;
    
    static constexpr bool kMergeSuffixOfSerializedStrings = true;
    static constexpr bool kUseBinaryLabel = false;
    using StringsMap = SerializedStrings<kUseBinaryLabel>;
    
    using BitVector = sim_ds::BitVector;
    
    friend class DoubleArrayCFSABuilder;
    using Builder = DoubleArrayCFSABuilder;
    
    using Explorer = AutomataExplorer;
    
    static std::string name() {
        return "DoubleArrayCFSA";
    }
    
    explicit DoubleArrayCFSA(const Builder& builder) {
        builder.Release(*this);
    }
    
    explicit DoubleArrayCFSA(const Builder& builder, ValueSet&& values) : DoubleArrayCFSA(builder) {
        values_ = std::move(values);
    }
    
    explicit DoubleArrayCFSA(const PlainFSA& fsa) {
        Builder builder(fsa);
        builder.Build(kUseBinaryLabel, kMergeSuffixOfSerializedStrings, !kUnionCheckAndId);
        builder.Release(*this);
    }
    
    explicit DoubleArrayCFSA(const PlainFSA& fsa, ValueSet&& values) : DoubleArrayCFSA(fsa) {
        values_ = std::move(values);
    }
    
    explicit DoubleArrayCFSA(std::istream& is) {
        LoadFrom(is);
    }
    
    bool Accept(std::string_view text) const {
        Explorer explorer(text);
        return Traverse_(explorer) && base_.is_final(explorer.trans());
    }
    
    id_type Lookup(std::string_view text) const;
    
    std::string Access(id_type key) const;
    
    CommonPrefixSet CommonPrefixSearch(std::string_view text) const {
        Explorer explorer(text);
        CommonPrefixSet prefixSet(text);
        size_t counter = 0;
        Traverse_(explorer, [&](auto exp) {
            auto trans = exp.trans();
            counter += base_.cum_words(trans);
            if (base_.is_final(trans)) { // Found prefix match one
                ++counter;
                prefixSet.AppendPrefixAndId(exp.pos() + 1, counter);
            }
        });
        return prefixSet;
    }
    
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
    
    void LoadFrom(std::istream& is) override {
        num_trans_ = read_val<size_t>(is);
        base_.LoadFrom(is);
        serialized_strings_.LoadFrom(is);
        if constexpr (kLinkChildren) {
            has_brother_bits_.Read(is);
            brother_ = read_vec<uint8_t>(is);
            is_node_bits_.Read(is);
            eldest_ = read_vec<uint8_t>(is);
        }
        values_.LoadFrom(is);
    }
    
    void StoreTo(std::ostream& os) const override {
        write_val(num_trans_, os);
        base_.StoreTo(os);
        serialized_strings_.StoreTo(os);
        if constexpr (kLinkChildren) {
            has_brother_bits_.Write(os);
            write_vec(brother_, os);
            is_node_bits_.Write(os);
            write_vec(eldest_, os);
        }
        values_.StoreTo(os);
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
        for (auto i = 0; i < 0x100; i++) {
            cout << i << '\t' << base_.is_final(i) << '\t' << base_.next(i) << '\t';
            if (!base_.is_string(i)) {
                cout << base_.check(i);
            } else {
                cout << serialized_strings_.string_view(base_.string_id(i));
            }
            if constexpr (kSupportAccess) {
                cout << '\t' << base_.words(i);
            }
            cout << endl;
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
    size_t num_trans_ = 0;
    Foundation base_;
    StringsMap serialized_strings_;
    // If use EDGE_LINK
    BitVector has_brother_bits_;
    std::vector<uint8_t> brother_;
    BitVector is_node_bits_;
    std::vector<uint8_t> eldest_;
    
    // If set values
    ValueSet values_;
    
    // MARK: Getter
    
    size_t target_state_(size_t index) const {
        return base_.next(index);
    }
    
    size_t transition_(size_t prev_trans, uint8_t label) const {
        auto state = target_state_(prev_trans);
        auto trans = state ^ label;
        assert(trans != prev_trans);
        return trans;
    }
    
    bool Traverse_(Explorer& explorer) const {
        return Traverse_(explorer, [](auto){});
    }
    
    template <class TransWork>
    bool Traverse_(Explorer& exp, TransWork trans_work) const {
        for (; exp.pos() < exp.text().size(); exp.set_pos(exp.pos() + 1)) {
            size_t i_text = exp.pos();
            uint8_t c = exp.text()[i_text];
            size_t trans = transition_(exp.trans(), c);
            if constexpr (kUnionCheckAndId) {
                if (!base_.is_string(trans)) {
                    // Check label that is character
                    uint8_t checkE = base_.check(trans);
                    if (checkE != c)
                        return false;
                } else {
                    // Check label that is indexed string
                    auto str_id = base_.string_id(trans);
                    if (!serialized_strings_.match(&i_text, exp.text(), str_id)) {
#ifndef NDEBUG
                        serialized_strings_.ShowLabels(str_id - 32, str_id + 32);
#endif
                        return false;
                    }
                }
            } else {
                uint8_t checkE = base_.check(trans);
                bool success_trans = checkE == c;
                if (base_.is_string(trans)) {
                    // Check label has indexed string
                    auto str_id = base_.string_id(trans);
                    bool success_trans_string = serialized_strings_.match(&(++i_text), exp.text(), str_id);
#ifndef NDEBUG
                    if (!success_trans_string) {
                        serialized_strings_.ShowLabels(str_id - 32, str_id + 32);
                    }
#endif
                    success_trans &= success_trans_string;
                }
                if (!success_trans)
                    return false;
            }
            
            exp.set_trans(trans);
            exp.set_pos(i_text);
            
            trans_work(exp);
        }
        
        return true;
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
    
    void transport_serialized_strings(StringsMap&& serizlized) {
        serialized_strings_ = std::move(serizlized);
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


template <bool UnionCheckAndId, bool UseCumulativeWords, bool LinkChildren, bool CompressStrId, bool CompressWords, bool SupportAccess>
id_type DoubleArrayCFSA<UnionCheckAndId, UseCumulativeWords, LinkChildren, CompressStrId, CompressWords, SupportAccess>::
Lookup(std::string_view text) const {
    // Separate algorithm from template parameters, that is to use cumulative-words.
    if constexpr (kUseCumulativeWords) {
        Explorer explorer(text);
        size_t counter = 0;
        bool traversed = Traverse_(explorer, [&](auto exp) {
            auto trans = exp.trans();
            counter += base_.cum_words(exp.trans());
            bool is_final_trans = base_.is_final(trans);
            if (is_final_trans) {
                counter++;
            }
            exp.observe(is_final_trans);
        });
        if (!traversed ||
            !(explorer.observed<bool>()) // Last translation doesn't arrive final state.
            ) {
            return kSearchError;
        }
        
        return counter;
        
    } else {
        size_t trans = 0;
        size_t counter = 0;
        for (size_t pos = 0, size = text.size(); pos < size; pos++) {
            if (trans > 0 && base_.is_final(trans))
                counter++;
            
            uint8_t c = text[pos];
            
            /*
            // Separate algorithm from template parameters, that is to use cumulative-words.
            if constexpr (C) {
                trans = transition_(trans, c);
                
                if constexpr (U) {
                    if (!base_.is_string(trans)) {
                        const uint8_t checkE = base_.check(trans);
                        if (checkE != c)
                            return kSearchError;
                    } else {
                        const auto sid = base_.string_id(trans);
                        if (!serialized_strings_.match(&pos, str, sid)) { // Increment 'pos'
#ifndef NDEBUG
                            serialized_strings_.ShowLabels(sid - 32, sid + 32);
#endif
                            return kSearchError;
                        }
                    }
                } else {
                    const uint8_t checkE = base_.check(trans);
                    bool success_trans = checkE == c;
                    if (base_.is_string(trans)) {
                        const auto sid = base_.string_id(trans);
                        bool success_trans_string = serialized_strings_.match(&(++pos), str, sid);
#ifndef NDEBUG
                        if (!success_trans_string) { // Increment 'pos'
                            serialized_strings_.ShowLabels(sid - 32, sid + 32);
                            return kSearchError;
                        }
#endif
                        success_trans &= success_trans_string;
                    }
                    if (!success_trans)
                        return kSearchError;
                }
                
                counter += base_.cum_words(trans);
            } else {
            */
            
            // Add counter to words at trans for each which has label at front is less than 'c'.
            for (uint8_t label = 1; label < c; label++) {
                size_t nt = transition_(trans, label);
                size_t check_type;
                
                if constexpr (kUnionCheckAndId) {
                    if (!base_.is_string(nt)) {
                        check_type = base_.check(nt);
                        if (check_type != label)
                            continue;
                    } else {
                        check_type = base_.string_id(nt);
                        if (serialized_strings_[check_type] != label)
                            continue;
                    }
                } else {
                    check_type = base_.check(nt);
                    if (check_type != label)
                        continue;
                }
                
                counter += base_.words(nt);
            }
            
            trans = transition_(trans, c);
            if constexpr (kUnionCheckAndId) {
                if (!base_.is_string(trans)) {
                    uint8_t checkE = base_.check(trans);
                    if (checkE != c)
                        return kSearchError;
                } else {
                    size_t str_id = base_.string_id(trans);
                    if (!serialized_strings_.match(&pos, text, str_id)) {
#ifndef NDEBUG
                        serialized_strings_.ShowLabels(str_id - 32, str_id + 32);
#endif
                        return kSearchError;
                    }
                }
            } else {
                uint8_t checkE = base_.check(trans);
                bool success_trans = checkE == c;
                
                if (base_.is_string(trans)) {
                    auto str_id = base_.string_id(trans);
                    if constexpr (!kUnionCheckAndId) {
                        pos++;
                    }
                    bool success_trans_string = serialized_strings_.match(&pos, text, str_id);
                    if (!success_trans_string) { // Increment 'pos'
#ifndef NDEBUG
                        serialized_strings_.ShowLabels(str_id - 32, str_id + 32);
#endif
                    }
                    success_trans &= success_trans_string;
                }
                if (!success_trans)
                    return kSearchError;
            }
            //            }
        }
        
        return base_.is_final(trans) ? ++counter : kSearchError;
    }
}


template <bool UnionCheckAndId, bool UseCumulativeWords, bool LinkChildren, bool CompressStrId, bool CompressWords, bool SupportAccess>
std::string DoubleArrayCFSA<UnionCheckAndId, UseCumulativeWords, LinkChildren, CompressStrId, CompressWords, SupportAccess>::
Access(id_type key) const {
    assert(kSupportAccess);
    
    size_t trans = 0;
    size_t counter = key;
    std::string str = "";
    while (counter > 0) {
        auto target_state = target_state_(trans);
        size_t next_trans = kSearchError;
        uint8_t c = (kLinkChildren ? eldest(target_state) : 1);
        while (true) {
            size_t nt = target_state ^ c;
            size_t check_type;
            
            bool success_trans;
            bool isStr = base_.is_string(nt);
            if constexpr (kUnionCheckAndId) {
                if (!isStr) {
                    check_type = base_.check(nt);
                    success_trans = check_type == c;
                } else {
                    check_type = base_.string_id(nt);
                    success_trans = uint8_t(serialized_strings_[check_type]) == c;
                }
            } else {
                uint8_t check_type = base_.check(nt);
                success_trans = check_type == c;
            }
            if (!success_trans) {
                if constexpr (kLinkChildren) {
                    return "";
                } else {
                    c++;
                    continue;
                }
            }
            
            auto curStore = base_.words(nt);
            if (curStore < counter) {
                counter -= curStore;
                if constexpr (kLinkChildren) {
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
                if constexpr (kUnionCheckAndId) {
                    if (!isStr)
                        str += check_type;
                    else
                        str += serialized_strings_.string_view(check_type);
                } else {
                    str += check_type;
                    if (isStr) {
                        size_t str_id = base_.string_id(nt);
                        str += serialized_strings_.string_view(str_id);
                    }
                }
                
                next_trans = nt;
                break;
            }
        }
        
        if (next_trans == kSearchError)
            return "";
        
        trans = next_trans;
    }
    
    return str;
    
}


}

#endif /* DoubleArrayCFSA_hpp */

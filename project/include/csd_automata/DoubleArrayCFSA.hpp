
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
    
    size_t* pos_ptr() {
        return &pos_on_text_;
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
    using Foundation = DAFoundation<kCompressNext, kUseStrId, kUnionCheckAndId, kCompressStrID, kHashing, kCompressWords, kUseCumulativeWords, kSupportAccess, kLinkChildren>;
    
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
    
    // MARK: - ByteData method
    
    size_t size_in_bytes() const override {
        auto size = sizeof(num_trans_);
        size += base_.size_in_bytes();
        size += strings_map_.size_in_bytes();
        size += values_.size_in_bytes();
        return size;
    }
    
    void LoadFrom(std::istream& is) override {
        num_trans_ = read_val<size_t>(is);
        base_.LoadFrom(is);
        strings_map_.LoadFrom(is);
        values_.LoadFrom(is);
    }
    
    void StoreTo(std::ostream& os) const override {
        write_val(num_trans_, os);
        base_.StoreTo(os);
        strings_map_.StoreTo(os);
        values_.StoreTo(os);
    }
    
    void ShowStats(std::ostream& os) const override {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl
        << "#trans:\t" << num_trans_ << endl
        << "#elems:\t" << base_.num_elements() << endl
        << "size:\t" << size_in_bytes() << endl;
        base_.ShowStats(os);
        os << "\tstrings:\t" << strings_map_.size_in_bytes() << endl;
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
                cout << strings_map_.string_view(base_.string_id(i));
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
    StringsMap strings_map_;
    // If set values in extended storage
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
            uint8_t c = exp.text()[exp.pos()];
            exp.set_trans(transition_(exp.trans(), c));
            if constexpr (kUnionCheckAndId) {
                if (!base_.is_string(exp.trans())) {
                    // Check label that is character
                    uint8_t checkE = base_.check(exp.trans());
                    if (checkE != c)
                        return false;
                } else {
                    // Check label that is indexed string
                    auto str_id = base_.string_id(exp.trans());
                    if (!strings_map_.match(exp.pos_ptr(), exp.text(), str_id)) {
#ifndef NDEBUG
                        strings_map_.ShowLabels(str_id - 32, str_id + 32);
#endif
                        return false;
                    }
                }
            } else {
                uint8_t checkE = base_.check(exp.trans());
                bool success_trans = checkE == c;
                if (base_.is_string(exp.trans())) {
                    // Check label has indexed string
                    auto str_id = base_.string_id(exp.trans());
                    exp.set_pos(exp.pos() + 1);
                    bool success_trans_string = strings_map_.match(exp.pos_ptr(), exp.text(), str_id);
#ifndef NDEBUG
                    if (!success_trans_string) {
                        strings_map_.ShowLabels(str_id - 32, str_id + 32);
                    }
#endif
                    success_trans &= success_trans_string;
                }
                if (!success_trans)
                    return false;
            }
            
            trans_work(exp);
        }
        
        return true;
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
            counter += base_.cum_words(exp.trans());
            bool is_final_trans = base_.is_final(exp.trans());
            if (is_final_trans)
                counter++;
            exp.observe(is_final_trans);
        });
        bool accept = traversed && explorer.observed<bool>();
        return accept ? counter : kSearchError;
        
    } else {
        size_t trans = 0;
        size_t counter = 0;
        for (size_t pos = 0, size = text.size(); pos < size; pos++) {
            if (trans > 0 && base_.is_final(trans))
                counter++;
            
            uint8_t c = text[pos];
            
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
                        if (strings_map_[check_type] != label)
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
                    if (!strings_map_.match(&pos, text, str_id)) {
#ifndef NDEBUG
                        strings_map_.ShowLabels(str_id - 32, str_id + 32);
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
                    bool success_trans_string = strings_map_.match(&pos, text, str_id);
                    if (!success_trans_string) { // Increment 'pos'
#ifndef NDEBUG
                        strings_map_.ShowLabels(str_id - 32, str_id + 32);
#endif
                    }
                    success_trans &= success_trans_string;
                }
                if (!success_trans)
                    return kSearchError;
            }
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
    std::string route = "";
    while (counter > 0) {
        auto target_state = target_state_(trans);
        size_t next_trans = kSearchError;
        uint8_t c = (kLinkChildren ? base_.eldest(target_state) : 1);
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
                    success_trans = uint8_t(strings_map_[check_type]) == c;
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
                    if (!base_.has_brother(nt))
                        return "";
                    c = base_.brother(nt);
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
                        route += check_type;
                    else
                        route += strings_map_.string_view(check_type);
                } else {
                    route += check_type;
                    if (isStr) {
                        size_t str_id = base_.string_id(nt);
                        route += strings_map_.string_view(str_id);
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
    
    return route;
    
}


}

#endif /* DoubleArrayCFSA_hpp */

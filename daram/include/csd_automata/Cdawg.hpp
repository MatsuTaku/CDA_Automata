
//  DoubleArrayCFSA.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/14.
//

#ifndef DoubleArrayCFSA_hpp
#define DoubleArrayCFSA_hpp

#include "StringDictionaryInterface.hpp"
#include "IOInterface.hpp"
#include "CdawgBuilder.hpp"

#include "DoubleArrayImpr.hpp"
#include "sim_ds/BitVector.hpp"
#include "SerializedStrings.hpp"
#include "ValueSet.hpp"

#include "CommonPrefixSet.hpp"

#include "sim_ds/log.hpp"


namespace csd_automata {


class Explorer {
public:
    Explorer(std::string_view text) : trans_(0), text_(text), pos_on_text_(0) {}
    
    void set_trans(size_t new_value) {
        trans_ = new_value;
    }
    
    size_t trans() const {
        return trans_;
    }
    
    std::string_view text() const {
        return text_;
    }
    
    void set_pos_on_text(size_t new_value) {
        pos_on_text_ = new_value;
    }
    
    size_t pos_on_text() const {
        return pos_on_text_;
    }
    
    size_t* pos_ptr() {
        return &pos_on_text_;
    }
    
    template <typename T>
    void observe(T new_value) {
        observe_target_ = new_value;
    }
    
    template <typename T = id_type>
    T observed() const {
        return T(observe_target_);
    }
    
private:
    size_t trans_;
    std::string_view text_;
    size_t pos_on_text_;
    id_type observe_target_ = 0;
};


template<bool UseCumulativeWords, bool LinkChildren, bool CompressStrId, bool CompressWords, bool SupportAccess, bool CompressNext, bool SelectStrId, bool DacWords>
class Cdawg : public StringDictionaryInterface, DoubleArrayImpr<CompressNext, true, CompressStrId, true, CompressWords, UseCumulativeWords, SupportAccess, LinkChildren, SelectStrId, DacWords> {
public:
    static_assert((SelectStrId and CompressStrId) or !SelectStrId, "ERROR: Failed template parameters: SelectStrId and CompressStrId");
    
    using Self = Cdawg<UseCumulativeWords, LinkChildren, CompressStrId, CompressWords, SupportAccess, CompressNext, SelectStrId, DacWords>;
    
    static constexpr bool kUseCumulativeWords = UseCumulativeWords;
    static constexpr bool kLinkChildren = LinkChildren;
    static constexpr bool kCompressStrId = CompressStrId;
    static constexpr bool kCompressWords = CompressWords;
    static constexpr bool kSupportAccess = SupportAccess;
    static constexpr bool kCompressNext = CompressNext;
    static constexpr bool kSelectStrId = SelectStrId;
    static constexpr bool kDacWords = DacWords;
    
    static constexpr id_type kHeader = (kUseCumulativeWords |
                                        kLinkChildren << 1 |
                                        kCompressStrId << 2 |
                                        kCompressWords << 3 |
                                        kSupportAccess << 4 |
                                        kCompressNext << 5 |
                                        kSelectStrId << 6 |
                                        kDacWords << 7);
    
    static constexpr bool kUseStrId = true;
    static constexpr bool kHashing = true;
    using Base = DoubleArrayImpr<kCompressNext, kUseStrId, kCompressStrId, kHashing, kCompressWords, kUseCumulativeWords, kSupportAccess, kLinkChildren, kSelectStrId, kDacWords>;
    
    static constexpr bool kMergeSuffixOfSerializedStrings = true;
    static constexpr bool kUseBinaryLabel = false;
    using StringsPool = SerializedStrings<kUseBinaryLabel, kSelectStrId>;
    
    using BitVector = sim_ds::BitVector;
    
    using Builder = CdawgBuilder;
    friend class CdawgBuilder;
    
    static constexpr size_t kSearchError = 0;
    
    static std::string name() {
        return typeid(Self).name();
    }
    
    static std::string tag() {
        return  (std::string("Daram-")
                 + (kSupportAccess ? "A" : "")
                 + (kCompressNext ? "N" : "")
                 + (kCompressStrId ? "C" : "")
                 + (kSelectStrId ? "s" : "")
                 + (kDacWords ? "Wd" : ""));
    }
    
private:
    size_t num_trans_ = 0;
    StringsPool strings_pool_;
    // If set values in extended storage
    ValueSet values_;
    
public:
    Cdawg() = default;
    
    explicit Cdawg(const Builder& builder) {
        builder.Release(*this);
    }
    
    explicit Cdawg(const Builder& builder, ValueSet&& values) : Cdawg(builder) {
        values_ = std::move(values);
    }
    
    explicit Cdawg(const PlainFSA& fsa) {
        Builder builder(fsa);
        builder.Build(kUseBinaryLabel, kMergeSuffixOfSerializedStrings);
        builder.Release(*this);
    }
    
    explicit Cdawg(const PlainFSA& fsa, ValueSet&& values) : Cdawg(fsa) {
        values_ = std::move(values);
    }
    
    explicit Cdawg(std::istream& is) {
        LoadFrom(is);
    }
    
    friend void LoadFromFile(Self& self, std::string file_name);
    
    bool Accept(std::string_view text) const override {
        Explorer explorer(text);
        return Traverse_(explorer) and Base::is_final(explorer.trans());
    }
    
    id_type Lookup(std::string_view text) const override {
        // Separate algorithm from template parameters, that is to use cumulative-words.
        if constexpr (kUseCumulativeWords) {
            Explorer explorer(text);
            size_t counter = 0;
            bool traversed = Traverse_(explorer, [&](auto& exp) {
                counter += Base::cum_words(exp.trans());
                bool is_final_trans = Base::is_final(exp.trans());
                if (is_final_trans)
                    ++counter;
                exp.observe(is_final_trans);
            });
            bool accept = traversed and explorer.observed<bool>();
            return accept ? counter : kSearchError;
        } else {
            return LookupLegacy(text);
        }
    }
    
    id_type LookupLegacy(std::string_view text) const;
    
    std::string Access(id_type key) const override;
    
    CommonPrefixSet CommonPrefixSearch(std::string_view text) const {
        Explorer explorer(text);
        CommonPrefixSet prefix_set(text);
        size_t counter = 0;
        Traverse_(explorer, [&](auto exp) {
            auto trans = exp.trans();
            counter += Base::cum_words(trans);
            if (Base::is_final(trans)) { // Found prefix match one
                ++counter;
                prefix_set.AppendPrefixAndId(exp.pos() + 1, counter);
            }
        });
        prefix_set.Freeze();
        return prefix_set;
    }
    
    // MARK: IOInterface method
    
    size_t size_in_bytes() const override {
        auto size = Base::size_in_bytes();
        size += sizeof(num_trans_);
        size += strings_pool_.size_in_bytes();
        size += values_.size_in_bytes();
        return size;
    }
    
    void LoadFrom(std::istream& is) override {
        auto header = read_val<id_type>(is);
        if (header != kHeader) {
            std::cerr << "ERROR: Class type is not match to stream! header: " << std::endl;
            sim_ds::ShowAsBinary(header);
            exit(EXIT_FAILURE);
        }
        
        Base::LoadFrom(is);
        num_trans_ = read_val<size_t>(is);
        strings_pool_.LoadFrom(is);
        values_.LoadFrom(is);
    }
    
    void StoreTo(std::ostream& os) const override {
        write_val(kHeader, os);
        
        Base::StoreTo(os);
        write_val(num_trans_, os);
        strings_pool_.StoreTo(os);
        values_.StoreTo(os);
    }
    
    void ShowStats(std::ostream& os) const override {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl
        << "#trans:\t" << num_trans_ << endl
        << "#elems:\t" << Base::num_elements() << endl
        << "size:\t" << size_in_bytes() << endl;
        Base::ShowStats(os);
        os << "\tstrings:\t" << strings_pool_.size_in_bytes() << endl;
        os << "\tsize values:\t" << values_.size_in_bytes() << endl;
    }
    
    void PrintForDebug(std::ostream& os) const {
        using std::cout, std::endl;
        cout << "id\tF\tN\tL\tCW" << endl;
        for (auto i = 0; i < 0x100; i++) {
            cout << i << '\t' << Base::is_final(i) << '\t' << Base::next(i) << '\t';
            if (!Base::is_string(i)) {
                cout << Base::check(i);
            } else {
                cout << strings_pool_.string_view(Base::string_id(i));
            }
            if constexpr (kSupportAccess) {
                cout << '\t' << Base::cum_words(i);
            }
            cout << endl;
        }
    }
    
private:
    size_t target_state_(size_t index) const {
        if constexpr (!kCompressNext) {
            return Base::next(index);
        } else {
            return Base::next(index) ^ index;
        }
    }
    
    size_t transition_(size_t prev_trans, uint8_t label) const {
        auto state = target_state_(prev_trans);
        auto trans = state ^ label;
        return trans;
    }
    
    bool Traverse_(Explorer& exp) const {
        return Traverse_(exp, [](auto){});
    }
    
    template <class TransWork>
    bool Traverse_(Explorer& exp, TransWork trans_work) const {
        for (; exp.pos_on_text() < exp.text().size(); exp.set_pos_on_text(exp.pos_on_text() + 1)) {
            uint8_t c = exp.text()[exp.pos_on_text()];
            exp.set_trans(transition_(exp.trans(), c));
            if (!Base::is_string(exp.trans())) {
                // Check label that is character
                uint8_t check = Base::check(exp.trans());
                if (check != c)
                    return false;
            } else {
                // Check label that is indexed string
                auto str_id = Base::string_id(exp.trans());
                if (!strings_pool_.match(exp.pos_ptr(), exp.text(), str_id)) {
#ifndef NDEBUG
                    strings_pool_.ShowLabels(str_id);
#endif
                    return false;
                }
            }
            
            trans_work(exp);
        }
        
        return true;
    }
    
};
    

template <bool UseCumulativeWords, bool LinkChildren, bool CompressStrId, bool CompressWords, bool SupportAccess, bool CompressNext, bool SelectStrId, bool DacWords>
void LoadFromFile(Cdawg<UseCumulativeWords, LinkChildren, CompressStrId, CompressWords, SupportAccess, CompressNext, SelectStrId, DacWords>& self, std::string file_name) {
    auto ifs = util::GetStreamOrDie<std::ifstream>(file_name);
    self.LoadFrom(ifs);
}


template <bool UseCumulativeWords, bool LinkChildren, bool CompressStrId, bool CompressWords, bool SupportAccess, bool CompressNext, bool SelectStrId, bool DacWords>
id_type Cdawg<UseCumulativeWords, LinkChildren, CompressStrId, CompressWords, SupportAccess, CompressNext, SelectStrId, DacWords>::
LookupLegacy(std::string_view text) const {
    assert(!kUseCumulativeWords);
    
    size_t trans = 0;
    size_t counter = 0;
    for (size_t pos = 0, size = text.size(); pos < size; pos++) {
        if (trans > 0 and Base::is_final(trans))
            counter++;
        
        uint8_t c = text[pos];
        
        // Add counter to words at trans for each which has label at front is less than 'c'.
        for (uint8_t label = 1; label < c; label++) {
            size_t nt = transition_(trans, label);
            size_t check_type;
            
            if (!Base::is_string(nt)) {
                check_type = Base::check(nt);
                if (check_type != label)
                    continue;
            } else {
                check_type = Base::string_id(nt);
                if (strings_pool_[check_type] != label)
                    continue;
            }
            
            counter += Base::words(nt);
        }
        
        trans = transition_(trans, c);
        if (!Base::is_string(trans)) {
            uint8_t checkE = Base::check(trans);
            if (checkE != c)
                return kSearchError;
        } else {
            size_t str_id = Base::string_id(trans);
            if (!strings_pool_.match(&pos, text, str_id)) {
#ifndef NDEBUG
                strings_pool_.ShowLabels(str_id);
#endif
                return kSearchError;
            }
        }
    }
    
    return Base::is_final(trans) ? ++counter : kSearchError;
}


template <bool UseCumulativeWords, bool LinkChildren, bool CompressStrId, bool CompressWords, bool SupportAccess, bool CompressNext, bool SelectStrId, bool DacWords>
std::string Cdawg<UseCumulativeWords, LinkChildren, CompressStrId, CompressWords, SupportAccess, CompressNext, SelectStrId, DacWords>::
Access(id_type key) const {
    assert(kSupportAccess);
    
    size_t trans = 0;
    size_t counter = key;
    std::string route = "";
    while (counter > 0) {
        auto target_state = target_state_(trans);
        size_t next_trans = kSearchError;
        uint8_t c = (kLinkChildren ? Base::eldest(target_state) : 1);
        while (true) {
            size_t nt = target_state ^ c;
            size_t check_type;
            
            bool success_trans;
            bool is_str_label = Base::is_string(nt);
            if (!is_str_label) {
                check_type = Base::check(nt);
                success_trans = check_type == c;
            } else {
                check_type = Base::string_id(nt);
                success_trans = uint8_t(strings_pool_[check_type]) == c;
            }
            if (!success_trans) {
                if constexpr (kLinkChildren) {
                    return "";
                } else {
                    c++;
                    continue;
                }
            }
            
            auto words = Base::words(nt);
            if (words < counter) {
                counter -= words;
                if constexpr (kLinkChildren) {
                    if (!Base::has_brother(nt))
                        return "";
                    c = Base::brother(nt);
                } else {
                    if (c == 0xff)
                        break;
                    c++;
                }
            } else {
                if (Base::is_final(nt))
                    counter--;
                if (!is_str_label)
                    route += check_type;
                else
                    route += strings_pool_.string_view(check_type);
                
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

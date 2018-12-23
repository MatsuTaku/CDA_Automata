//
//  DAFoundation.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/13.
//

#ifndef NextCheck_hpp
#define NextCheck_hpp

#include "IOInterface.hpp"
#include "MultipleVector.hpp"
#include "sim_ds/BitVector.hpp"
#include "sim_ds/DacVector.hpp"
#include "sim_ds/FitVector.hpp"

#include "sim_ds/bit_tools.hpp"
#include "sim_ds/calc.hpp"

namespace csd_automata {

template <bool CompNext, bool UseStrId, bool UnionCheckAndId, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords, bool LinkChildren>
class DAFoundation : MultipleVector {
public:
    using Self = DAFoundation<CompNext, UseStrId, UnionCheckAndId, CompId, Hashing, CompWords, CumuWords, PlainWords, LinkChildren>;
    using Base = MultipleVector;
    
    static constexpr bool kCompressNext = CompNext;
    static constexpr bool kUseStrId = UseStrId;
    static constexpr bool kUnionCheckAndId = UnionCheckAndId;
    static constexpr bool kCompressStrId = CompId;
    static constexpr bool kHashing = Hashing;
    static constexpr bool kCompressWords = CompWords;
    static constexpr bool kCumulatesWords = CumuWords;
    static constexpr bool kPlainWords = PlainWords;
    static constexpr bool kLinkChildren = LinkChildren;
    
    enum Bits : size_t {
        kFlags = !kCompressNext ? (kCompressStrId ? 2 : 1) : 0,
        kFlagsExtend = kCompressNext ? (kCompressStrId ? 2 : 1) : 0,
        kBitsUpperNext = 8,
        kBitsUpperWords = kPlainWords ? 4 : 8,
    };
    enum Mask : Base::storage_type {
        kMaskIsFinal = 0b01,
        kMaskIsStrId = 0b10,
        kMaskFlags = sim_ds::bit_tools::width_mask<kFlags>,
        kMaskUpperNext = sim_ds::bit_tools::width_mask<kBitsUpperNext>,
        kMaskUpperWords = sim_ds::bit_tools::width_mask<kBitsUpperWords>,
    };
    enum ElementNumber : size_t {
        kElementIdCheck = 0,
        kElementIdNext = 1,
        kElementIdCWords = 2,
        kElementIdWords = 3,
    };
    enum ElementPosition : size_t {
        kElementPositionNext = 1,
        kElementPositionCheck = 0,
    };
    
    using BitVector = sim_ds::BitVector;
    using DacVector = sim_ds::DacVector;
    using FitVector = sim_ds::FitVector;
    
    size_t next(size_t index) const {
        size_t ne = Base::get_(Base::offset_(index) + kElementPositionNext, Base::element_table_[kElementIdNext].size);
        if constexpr (!kCompressNext) {
            return ne >> kFlags;
        } else {
            if (!next_link_bits_[index]) {
                return ne;
            } else {
                auto rank = next_link_bits_.rank(index);
                return ne | (next_flow_[rank] << kBitsUpperNext);
            }
        }
    }
    
    void set_next(size_t index, size_t next) {
        if constexpr (!kCompressNext) {
            auto flags = unmasked_flags_(index) & kMaskFlags;
            Base::set_nested_element<kElementIdNext>(index, next << kFlags | flags);
        } else {
            Base::set_nested_element<kElementIdNext>(index, next & kMaskUpperNext);
            id_type flow = next >> kBitsUpperNext;
            bool has_flow = flow > 0;
            next_link_bits_[index] = has_flow;
            if (has_flow) {
                next_flow_src_.push_back(flow);
            }
        }
    }
    
    uint8_t check(size_t index) const {
        return Base::bytes_[Base::offset_(index) + kElementPositionCheck];
    }
    
    void set_check(size_t index, uint8_t check) {
        Base::set_nested_element<kElementIdCheck>(index, check);
    }
    
    bool is_final(size_t index) const {
        if constexpr (!kCompressNext) {
            return static_cast<bool>(unmasked_flags_(index) & kMaskIsFinal);
        } else {
            return flags_bits_[index * kFlagsExtend];
        }
    }
    
    void set_is_final(size_t index, bool is_final) {
        if constexpr (!kCompressNext) {
            auto v = unmasked_flags_(index);
            if (is_final)
                v |= kMaskIsFinal;
            else
                v &= ~kMaskIsFinal;
            Base::block(index).template restricted_set<kElementIdNext, uint8_t>(v);
        } else {
            flags_bits_[index * kFlagsExtend] = is_final;
        }
    }
    
    bool is_string(size_t index) const {
        if constexpr (kCompressStrId) {
            if constexpr (!kCompressNext) {
                return static_cast<bool>(unmasked_flags_(index) & kMaskIsStrId);
            } else {
                return flags_bits_[index * kFlagsExtend + 1];
            }
        } else {
            return check_link_bits_[index];
        }
    }
    
    void set_is_string(size_t index, bool is_str) {
        assert(kUseStrId);
        if constexpr (kCompressStrId) {
            if constexpr (!kCompressNext) {
                auto v = unmasked_flags_(index);
                if (is_str)
                    v |= kMaskIsStrId;
                else
                    v &= ~kMaskIsStrId;
                Base::block(index).template restricted_set<kElementIdNext, uint8_t>(v);
            } else {
                flags_bits_[index * kFlagsExtend + 1] = is_str;
            }
        } else {
            check_link_bits_[index] = is_str;
        }
    }
    
    size_t string_id(size_t index) const {
        assert(kUseStrId);
        if constexpr (kUnionCheckAndId) {
            auto id = check(index);
            if (kCompressStrId && !check_link_bits_[index]) {
                return id;
            } else {
                auto rank = check_link_bits_.rank(index);
                return id | (check_flow_[rank] << kBitsUpperNext);
            }
        } else {
            assert(!kCompressStrId);
            auto rank = check_link_bits_.rank(index);
            return check_flow_[rank];
        }
    }
    
    void set_string_id(size_t index, size_t str_id) {
        assert(kUseStrId);
        
        if constexpr (kUnionCheckAndId) {
            set_check(index, str_id & 0xff);
            auto flow = str_id >> kBitsUpperNext;
            bool has_flow = flow > 0;
            if constexpr (kCompressStrId)
                check_link_bits_[index] = has_flow;
            if (!kCompressStrId || (kCompressStrId && has_flow))
                check_flow_src_.push_back(flow);
        } else {
            assert(!kCompressStrId);
            check_flow_src_.emplace_back(str_id);
        }
    }
    
    size_t cum_words(size_t index) const {
        assert(kHashing);
        assert(kCumulatesWords);
        if constexpr (kCompressWords) {
            if constexpr (kPlainWords) {
                size_t cw = front_byte_of_element<kElementIdCWords>(index) & kMaskUpperWords;
                if (!cum_words_link_bits_[index])
                    return cw;
                else {
                    auto rank = cum_words_link_bits_.rank(index);
                    return cw | (cum_words_flow_[rank] << kBitsUpperWords);
                }
            } else {
                size_t cw = front_byte_of_element<kElementIdCWords>(index);
                if (!cum_words_link_bits_[index])
                    return cw;
                else {
                    auto rank = cum_words_link_bits_.rank(index);
                    return cw | (cum_words_flow_[rank] << kBitsUpperWords);
                }
            }
        } else {
            return Base::nested_element<kElementIdCWords>(index);
        }
    }
    
    void set_cum_words(size_t index, size_t cw) {
        assert(kHashing);
        assert(kCumulatesWords);
        
        if constexpr (kCompressWords) {
            if constexpr (kPlainWords) {
                // Shared same byte with words to save each upper 4bits.
                uint8_t base = Base::nested_element<kElementIdCWords>(index);
                base &= kMaskUpperWords << kBitsUpperWords;
                base |= cw & kMaskUpperWords;
                Base::set_nested_element<kElementIdCWords>(index, base);
                auto flow = cw >> kBitsUpperWords;
                bool has_flow = flow > 0;
                cum_words_link_bits_[index] = has_flow;
                if (has_flow)
                    cum_words_flow_src_.push_back(flow);
            } else {
                Base::set_nested_element<kElementIdCWords>(index, cw & kMaskUpperWords);
                auto flow = cw >> kBitsUpperWords;
                bool hasFlow = flow > 0;
                cum_words_link_bits_[index] = hasFlow;
                if (hasFlow)
                    cum_words_flow_src_.push_back(flow);
            }
        } else {
            Base::set_nested_element<kElementIdCWords>(index, cw);
        }
    }
    
    size_t words(size_t index) const {
        assert(kHashing);
        assert(kPlainWords);
        if constexpr (kCompressWords) {
            size_t words = front_byte_of_element<kElementIdCWords>(index) >> kBitsUpperWords;
            if (!words_link_bits_[index])
                return words;
            else {
                auto rank = words_link_bits_.rank(index);
                return words | (words_flow_[rank] << kBitsUpperWords);
            }
        } else {
            return Base::nested_element<kElementIdWords>(index);
        }
    }
    
    void set_words(size_t index, size_t words) {
        assert(kHashing);
        assert(kPlainWords);
        
        if constexpr (kCompressWords) {
            // Shared same byte with c-words to save each upper 4bits.
            uint8_t base = Base::nested_element<kElementIdCWords>(index);
            base &= kMaskUpperWords;
            base |= ((words & kMaskUpperWords) << kBitsUpperWords);
            Base::set_nested_element<kElementIdCWords>(index, base);
            auto flow = words >> kBitsUpperWords;
            bool has_flow = flow > 0;
            words_link_bits_[index] = has_flow;
            if (has_flow)
                words_flow_src_.push_back(flow);
        } else {
            Base::set_nested_element<kElementIdWords>(index, words);
        }
    }
    
    bool has_brother(size_t index) const {
        assert(kLinkChildren);
        return has_brother_bits_[index];
    }
    
    void set_has_brother(size_t index, bool has) {
        has_brother_bits_[index] = has;
    }
    
    uint8_t brother(size_t index) const {
        assert(kLinkChildren);
        assert(has_brother_bits_[index]);
        return brother_[has_brother_bits_.rank(index)];
    }
    
    void set_brother(size_t index, uint8_t bro) {
        brother_.emplace_back(bro);
    }
    
    bool is_state(size_t index) const {
        assert(kLinkChildren);
        return is_state_bits_[index];
    }
    
    void set_is_state(size_t index, bool is_state) {
        is_state_bits_[index] = is_state;
    }
    
    uint8_t eldest(size_t index) const {
        assert(kLinkChildren);
        assert(is_state_bits_[index]);
        return eldest_[is_state_bits_.rank(index)];
    }
    
    void set_eldest(size_t index, uint8_t eldest) {
        eldest_.emplace_back(eldest);
    }
    
    size_t num_elements() const {
        return Base::size();
    }
    
    // MARK: - Protocol settings for build
    
    // First. Set size of elements
    void resize(size_t size, size_t words = 0) {
        auto nextSize = sim_ds::calc::SizeFitsInBytes(size << kFlags);
        std::vector<size_t> element_sizes = {1, kCompressNext ? 1 : nextSize};
        if constexpr (kHashing) {
            if constexpr (kCompressWords) {
                element_sizes.push_back(1);
            } else {
                auto words_size = sim_ds::calc::SizeFitsInBytes(words);
                element_sizes.push_back(words_size);
                if (kPlainWords)
                    element_sizes.push_back(words_size);
            }
        }
        Base::set_element_sizes(element_sizes);
        Base::resize(size);
        if constexpr (kCompressNext) {
            next_link_bits_.resize(size);
            flags_bits_.resize(size * kFlagsExtend);
        }
        if constexpr (kUseStrId)
            check_link_bits_.resize(size);
        if constexpr (kHashing && kCompressWords) {
            cum_words_link_bits_.resize(size);
            if constexpr (kPlainWords)
                words_link_bits_.resize(size);
        }
        if constexpr (kLinkChildren) {
            has_brother_bits_.resize(size);
            is_state_bits_.resize(size);
        }
    }
    
    // Finaly. Serialize flows.
    void Build() {
        if constexpr (kCompressNext) {
            next_link_bits_.Build();
            next_flow_ = DacVector(next_flow_src_);
            next_flow_src_.resize(0);
            next_flow_src_.shrink_to_fit();
        }
        if constexpr (kUseStrId) {
            check_link_bits_.Build();
            check_flow_ = FitVector(check_flow_src_);
            check_flow_src_.resize(0);
            check_flow_src_.shrink_to_fit();
        }
        if constexpr (kHashing) {
            if constexpr (kPlainWords) {
                words_link_bits_.Build();
                words_flow_ = FitVector(words_flow_src_);
                words_flow_src_.resize(0);
                words_flow_src_.shrink_to_fit();
            }
            cum_words_link_bits_.Build();
            cum_words_flow_ = FitVector(cum_words_flow_src_);
            cum_words_flow_src_.resize(0);
            cum_words_flow_src_.shrink_to_fit();
        }
        if constexpr (kLinkChildren) {
            has_brother_bits_.Build();
            is_state_bits_.Build();
        }
    }
    
    // MARK: - IO
    
    size_t size_in_bytes() const override {
        auto size = Base::size_in_bytes();
        if constexpr (kCompressNext) {
            size += next_link_bits_.size_in_bytes();
            size += next_flow_.size_in_bytes();
            size += flags_bits_.size_in_bytes();
        }
        if constexpr (kUseStrId) {
            size += check_link_bits_.size_in_bytes();
            size += check_flow_.size_in_bytes();
        }
        if constexpr (kHashing && kCompressWords) {
            if constexpr (kPlainWords) {
                size += words_link_bits_.size_in_bytes();
                size += words_flow_.size_in_bytes();
            }
            if constexpr (kCumulatesWords) {
                size += cum_words_link_bits_.size_in_bytes();
                size += cum_words_flow_.size_in_bytes();
            }
        }
        if constexpr (kLinkChildren) {
            size += has_brother_bits_.size_in_bytes();
            size += size_vec(brother_);
            size += is_state_bits_.size_in_bytes();
            size += size_vec(eldest_);
        }
        return size;
    }
    
    void LoadFrom(std::istream& is) override {
        Base::LoadFrom(is);
        if constexpr (kCompressNext) {
            next_link_bits_.Read(is);
            next_flow_.Read(is);
            flags_bits_.Read(is);
        }
        if constexpr (kUseStrId) {
            check_link_bits_.Read(is);
            check_flow_ = FitVector(is);
        }
        if constexpr (kHashing && kCompressWords) {
            if constexpr (kPlainWords) {
                words_link_bits_.Read(is);
                words_flow_ = FitVector(is);
            }
            if constexpr (kCumulatesWords) {
                cum_words_link_bits_.Read(is);
                cum_words_flow_ = FitVector(is);
            }
        }
        if constexpr (kLinkChildren) {
            has_brother_bits_.Read(is);
            brother_ = read_vec<uint8_t>(is);
            is_state_bits_.Read(is);
            eldest_ = read_vec<uint8_t>(is);
        }
    }
    
    void StoreTo(std::ostream& os) const override {
        Base::StoreTo(os);
        if constexpr (kCompressNext) {
            next_link_bits_.Write(os);
            next_flow_.Write(os);
            flags_bits_.Write(os);
        }
        if constexpr (kUseStrId) {
            check_link_bits_.Write(os);
            check_flow_.Write(os);
        }
        if constexpr (kHashing && kCompressWords) {
            if constexpr (kPlainWords) {
                words_link_bits_.Write(os);
                words_flow_.Write(os);
            }
            if constexpr (kCumulatesWords) {
                cum_words_link_bits_.Write(os);
                cum_words_flow_.Write(os);
            }
        }
        if constexpr (kLinkChildren) {
            has_brother_bits_.Write(os);
            write_vec(brother_, os);
            is_state_bits_.Write(os);
            write_vec(eldest_, os);
        }
    }
    
    void ShowStats(std::ostream& os) const override;
    
    void ShowSizeMap(std::ostream& os) const;
    
    // MARK: Copy guard
    
    DAFoundation() = default;
    ~DAFoundation() = default;
    
    DAFoundation (const DAFoundation&) = delete;
    DAFoundation& operator=(const DAFoundation&) = delete;
    
    DAFoundation (DAFoundation&&) noexcept = default;
    DAFoundation& operator=(DAFoundation&&) noexcept = default;
    
private:
    // Enabled at NextCheck<true, C>
    BitVector next_link_bits_;
    DacVector next_flow_;
    BitVector flags_bits_;
    // Enabled at NextCheck<N, true>
    BitVector check_link_bits_;
    FitVector check_flow_;
    // Enable ar NextCheck<N, C, true, true, CW>
    BitVector words_link_bits_;
    FitVector words_flow_;
    BitVector cum_words_link_bits_;
    FitVector cum_words_flow_;
    // If use LinkChildren
    BitVector has_brother_bits_;
    std::vector<uint8_t> brother_;
    BitVector is_state_bits_;
    std::vector<uint8_t> eldest_;
    
    // For build
    using array_type = std::vector<id_type>;
    array_type next_flow_src_;
    array_type check_flow_src_;
    array_type words_flow_src_;
    array_type cum_words_flow_src_;
    
    template <size_t Id>
    Base::storage_type front_byte_of_element(size_t index) const {
        auto relative_pos = Base::element_table_[Id].pos;
        return Base::bytes_[Base::offset_(index) + relative_pos];
    }
    
    /**
     Flags that is_final and is_string_label are stored in upper bits of next.
     */
    uint8_t unmasked_flags_(size_t index) const {
        assert(!kCompressNext);
        return Base::bytes_[offset_(index) + kElementPositionNext];
    }
    
};


template<bool CompNext, bool CompCheck, bool UnionCheckAndId, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords, bool LinkChildren>
void DAFoundation<CompNext, CompCheck, UnionCheckAndId, CompId, Hashing, CompWords, CumuWords, PlainWords, LinkChildren>::
ShowStats(std::ostream& os) const {
    using std::endl;
    auto codes_name = [](bool use) {
        return use ? "Comp" : "Plain";
    };
    os << "--- Stat of " << "DAFoundation N:" << codes_name(kCompressNext) << "|C:" << codes_name(kUseStrId) << " ---" << endl;
    os << "size:\t" << size_in_bytes() << endl;
    os << "\tbytes:\t" << Base::size_in_bytes() << endl;
    os << "\tnext:\t" << num_elements() * Base::element_size(kElementIdNext) + next_link_bits_.size_in_bytes() +  next_flow_.size_in_bytes() << endl;
    os << "\tcheck:\t" << num_elements() + check_link_bits_.size_in_bytes() + check_flow_.size_in_bytes() << endl;
    if constexpr (kHashing) {
        size_t cWordsSize;
        if constexpr (kPlainWords) {
            size_t wordsSize;
            if constexpr (kCumulatesWords) {
                wordsSize = num_elements() / 2 + words_link_bits_.size_in_bytes() + words_flow_.size_in_bytes();
                cWordsSize = num_elements() / 2 + cum_words_link_bits_.size_in_bytes() + cum_words_flow_.size_in_bytes();
            } else {
                wordsSize = num_elements() * Base::element_size(kElementIdWords);
                cWordsSize = num_elements() * Base::element_size(kElementIdCWords);
            }
            os << "\twords:\t" << wordsSize << endl;
        } else {
            cWordsSize = kCumulatesWords ? (num_elements() + cum_words_link_bits_.size_in_bytes() + cum_words_flow_.size_in_bytes()) : (num_elements() * Base::element_size(kElementIdCWords));
        }
        os << "\tcum_words:\t" << cWordsSize << endl;
        
        os << "---  ---" << endl;
    }
    if constexpr (kLinkChildren) {
        os << "\tbrother:\t" << has_brother_bits_.size_in_bytes() + size_vec(brother_) << endl;
        os << "\teldest:\t" << is_state_bits_.size_in_bytes() + size_vec(eldest_) << endl;
        os << "---  ---" << endl;
    }
    //        showSizeMap(os);
}

    
template<bool CompNext, bool CompCheck, bool UnionCheckAndId, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords, bool LinkChildren>
void DAFoundation<CompNext, CompCheck, UnionCheckAndId, CompId, Hashing, CompWords, CumuWords, PlainWords, LinkChildren>::
ShowSizeMap(std::ostream& os) const {
    auto numElem = num_elements();
    std::vector<size_t> nexts(numElem);
    for (auto i = 0; i < numElem; i++)
        nexts[i] = next(i) >> (!kCompressNext ? 1 : 0);
    
    auto showList = [&](const std::vector<size_t>& list) {
        using std::endl;
        os << "-- " << "Next Map" << " --" << endl;
        for (auto c : list)
            os << c << "\t" << endl;
        os << "/ " << numElem << endl;
    };
    
}
    
}

#endif /* NextCheck_hpp */

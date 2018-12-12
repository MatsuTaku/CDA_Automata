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
    
using sim_ds::bit_tools::width_mask;

template <bool CompNext, bool UseStrId, bool UnionCheckAndId, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords>
class DAFoundation : IOInterface {
public:
    using Self = DAFoundation<CompNext, UseStrId, UnionCheckAndId, CompId, Hashing, CompWords, CumuWords, PlainWords>;
    
    static constexpr bool kCompressNext = CompNext;
    static constexpr bool kUseStrId = UseStrId;
    static constexpr bool kUnionCheckAndId = UnionCheckAndId;
    static constexpr bool kCompressStrId = CompId;
    static constexpr bool kHashing = Hashing;
    static constexpr bool kCompressWords = CompWords;
    static constexpr bool kCumulatesWords = CumuWords;
    static constexpr bool kPlainWords = PlainWords;
    
    static constexpr id_type kMaskAccept = 0b01;
    static constexpr id_type kMaskIsStrId = 0b10;
    static constexpr size_t kFlags = !kCompressNext ? (kCompressStrId ? 2 : 1) : 0;
    static constexpr size_t kBitsUpperNext = 8;
    static constexpr id_type kMaskUpperNext = width_mask<kBitsUpperNext>;
    static constexpr size_t kBitsUpperWords = kPlainWords ? 4 : 8;
    static constexpr id_type kMaskUpperWords = width_mask<kBitsUpperWords>;
    enum ElementNumber {
        kENNext = 0,
        kENCheck = 1,
        kENCWords = 2,
        kENWords = 3,
    };
    
    using BitVector = sim_ds::BitVector;
    using DacVector = sim_ds::DacVector;
    using FitVector = sim_ds::FitVector;
    
    DAFoundation(size_t size, size_t num_queries = 0) {
        resize(size, num_queries);
    }
    
    size_t next(size_t index) const {
        size_t ne = multiple_base_.nested_element<kENNext>(index);
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
    
    uint8_t check(size_t index) const {
        return static_cast<uint8_t>(multiple_base_.nested_element<kENCheck>(index));
    }
    
    bool is_string(size_t index) const {
        assert(!kCompressNext);
        if constexpr (kCompressStrId) {
            return static_cast<bool>(unmasked_flags_(index) & kMaskIsStrId);
        } else {
            return check_link_bits_[index];
        }
    }
    
    bool is_final(size_t index) const {
        assert(!kCompressNext);
        return static_cast<bool>(unmasked_flags_(index) & kMaskAccept);
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
    
    size_t cum_words(size_t index) const {
        assert(kHashing);
        assert(kCumulatesWords);
        if constexpr (kCompressWords) {
            if constexpr (kPlainWords) {
                size_t cw = multiple_base_.nested_element<kENCWords>(index) & kMaskUpperWords;
                if (!cum_words_link_bits_[index])
                    return cw;
                else {
                    auto rank = cum_words_link_bits_.rank(index);
                    return cw | (cum_words_flow_[rank] << kBitsUpperWords);
                }
            } else {
                size_t cw = multiple_base_.nested_element<kENCWords>(index);
                if (!cum_words_link_bits_[index])
                    return cw;
                else {
                    auto rank = cum_words_link_bits_.rank(index);
                    return cw | (cum_words_flow_[rank] << kBitsUpperWords);
                }
            }
        } else {
            return multiple_base_.nested_element<kENCWords>(index);
        }
        
    }
    
    size_t words(size_t index) const {
        assert(kHashing);
        assert(kPlainWords);
        if constexpr (kCompressWords) {
            size_t words = multiple_base_.nested_element<kENCWords>(index) >> kBitsUpperWords;
            if (!words_link_bits_[index])
                return words;
            else {
                auto rank = words_link_bits_.rank(index);
                return words | (words_flow_[rank] << kBitsUpperWords);
            }
        } else {
            return multiple_base_.nested_element<kENWords>(index);
        }
    }
    
    size_t num_elements() const {
        return multiple_base_.size();
    }
    
    // MARK: - Protocol settings for build
    
    // First. Set size of elements
    void resize(size_t size, size_t words = 0) {
        auto nextSize = sim_ds::calc::SizeFitsInBytes(size << kFlags);
        std::vector<size_t> sizes = { kCompressNext ? 1 : nextSize, 1 };
        if constexpr (kHashing) {
            if constexpr (kCompressWords) {
                sizes.push_back(1);
            } else {
                auto wordsSize = sim_ds::calc::SizeFitsInBytes(words);
                sizes.push_back(wordsSize);
                if (kPlainWords)
                    sizes.push_back(wordsSize);
            }
        }
        multiple_base_.set_value_sizes(sizes);
        multiple_base_.resize(size);
        if (kCompressNext) next_link_bits_.resize(size);
        if (kUseStrId) check_link_bits_.resize(size);
        if (kHashing && kCompressWords) {
            cum_words_link_bits_.resize(size);
            if (kPlainWords)
                words_link_bits_.resize(size);
        }
    }
    
    void set_is_final(size_t index, bool is_final) {
        assert(!kCompressNext);
        auto v = unmasked_flags_(index);
        if (is_final)
            v |= kMaskAccept;
        else
            v &= ~kMaskAccept;
        multiple_base_.block(index).template restricted_set<kENNext, uint8_t>(v);
    }
    
    void set_is_string(size_t index, bool is_str) {
        assert(kUseStrId);
        if constexpr (kCompressStrId) {
            auto v = unmasked_flags_(index);
            if (is_str)
                v |= kMaskIsStrId;
            else
                v &= ~kMaskIsStrId;
            multiple_base_.block(index).template restricted_set<kENNext, uint8_t>(v);
        } else {
            check_link_bits_[index] = is_str;
        }
    }
    
    void set_next(size_t index, size_t next) {
        if constexpr (!kCompressNext) {
            auto flags = unmasked_flags_(index) & width_mask<kFlags>;
            multiple_base_.set_nested_element<kENNext>(index, next << kFlags | flags);
        } else {
            multiple_base_.set_nested_element<kENNext>(index, next & kMaskUpperNext);
            auto flow = next >> kBitsUpperNext;
            bool hasFlow = flow > 0;
            next_link_bits_[index] = hasFlow;
            if (hasFlow)
                next_flow_src_.emplace_back(flow);
        }
        
    }
    
    void set_check(size_t index, uint8_t check) {
        multiple_base_.set_nested_element<kENCheck>(index, check);
    }
    
    void set_string_id(size_t index, size_t str_id) {
        assert(kUseStrId);
        
        if constexpr (kUnionCheckAndId) {
            set_check(index, str_id & 0xff);
            auto flow = str_id >> kBitsUpperNext;
            bool hasFlow = flow > 0;
            if constexpr (kCompressStrId)
                check_link_bits_[index] = hasFlow;
            if (!kCompressStrId || (kCompressStrId && hasFlow))
                check_flow_src_.emplace_back(flow);
        } else {
            assert(!kCompressStrId);
            check_flow_src_.emplace_back(str_id);
        }
    }
    
    void set_cum_words(size_t index, size_t cw) {
        assert(kHashing);
        assert(kCumulatesWords);
        
        if constexpr (kCompressWords) {
            if constexpr (kPlainWords) {
                // Shared same byte with words to save each upper 4bits.
                uint8_t base = multiple_base_.nested_element<kENCWords>(index);
                base &= kMaskUpperWords << kBitsUpperWords;
                base |= cw & kMaskUpperWords;
                multiple_base_.set_nested_element<kENCWords>(index, base);
                auto flow = cw >> kBitsUpperWords;
                bool hasFlow = flow > 0;
                cum_words_link_bits_[index] = hasFlow;
                if (hasFlow)
                    cum_words_flow_src_.emplace_back(flow);
            } else {
                multiple_base_.set_nested_element<kENCWords>(index, cw & kMaskUpperWords);
                auto flow = cw >> kBitsUpperWords;
                bool hasFlow = flow > 0;
                cum_words_link_bits_[index] = hasFlow;
                if (hasFlow)
                    cum_words_flow_src_.emplace_back(flow);
            }
        } else {
            multiple_base_.set_nested_element<kENCWords>(index, cw);
        }
    }
    
    void set_words(size_t index, size_t words) {
        assert(kHashing);
        assert(kPlainWords);
        
        if constexpr (kCompressWords) {
            // Shared same byte with c-words to save each upper 4bits.
            uint8_t base = multiple_base_.nested_element<kENCWords>(index);
            base &= kMaskUpperWords;
            base |= ((words & kMaskUpperWords) << kBitsUpperWords);
            multiple_base_.set_nested_element<kENCWords>(index, base);
            auto flow = words >> kBitsUpperWords;
            bool hasFlow = flow > 0;
            words_link_bits_[index] = hasFlow;
            if (hasFlow)
                words_flow_src_.emplace_back(flow);
        } else {
            multiple_base_.set_nested_element<kENWords>(index, words);
        }
    }
    
    // Finaly. Serialize flows.
    void Build() {
        if constexpr (kCompressNext) {
            next_link_bits_.Build();
            next_flow_.ConvertFromVector(next_flow_src_);
            next_flow_.Build();
        }
        if constexpr (kUseStrId) {
            check_link_bits_.Build();
            check_flow_ = FitVector(check_flow_src_);
        }
        if constexpr (kHashing) {
            if constexpr (kPlainWords) {
                words_link_bits_.Build();
                words_flow_ = FitVector(words_flow_src_);
            }
            cum_words_link_bits_.Build();
            cum_words_flow_ = FitVector(cum_words_flow_src_);
        }
    }
    
    // MARK: - IO
    
    size_t size_in_bytes() const override {
        auto size = multiple_base_.size_in_bytes();
        if constexpr (kCompressNext) {
            size += next_link_bits_.size_in_bytes();
            size += next_flow_.size_in_bytes();
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
        return size;
    }
    
    void StoreTo(std::ostream& os) const override {
        multiple_base_.StoreTo(os);
        if constexpr (kCompressNext) {
            next_link_bits_.Write(os);
            next_flow_.Write(os);
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
    }
    
    void LoadFrom(std::istream& is) override {
        multiple_base_.LoadFrom(is);
        if constexpr (kCompressNext) {
            next_link_bits_.Read(is);
            next_flow_.Read(is);
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
    }
    
    void ShowStats(std::ostream& os) const override;
    
    void ShowSizeMap(std::ostream& os) const;
    
    // MARK: Copy guard
    
    DAFoundation() = default;
    ~DAFoundation() = default;
    
    DAFoundation (const DAFoundation&) = delete;
    DAFoundation& operator =(const DAFoundation&) = delete;
    
    DAFoundation (DAFoundation&&) noexcept = default;
    DAFoundation& operator =(DAFoundation&&) noexcept = default;
    
private:
    MultipleVector multiple_base_;
    // Enabled at NextCheck<true, C>
    BitVector next_link_bits_;
    DacVector next_flow_;
    // Enabled at NextCheck<N, true>
    BitVector check_link_bits_;
    FitVector check_flow_;
    // Enable ar NextCheck<N, C, true, true, CW>
    BitVector words_link_bits_;
    FitVector words_flow_;
    BitVector cum_words_link_bits_;
    FitVector cum_words_flow_;
    
    // For build
    using array_type = std::vector<size_t>;
    array_type next_flow_src_;
    array_type check_flow_src_;
    array_type words_flow_src_;
    array_type cum_words_flow_src_;
    
    uint8_t unmasked_flags_(size_t index) const {
        assert(!kCompressNext);
        return multiple_base_.block(index).template restricted_get<kENNext, uint8_t>();
    }
    
};


template<bool CompNext, bool CompCheck, bool UnionCheckAndId, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords>
void DAFoundation<CompNext, CompCheck, UnionCheckAndId, CompId, Hashing, CompWords, CumuWords, PlainWords>::
ShowStats(std::ostream& os) const {
    using std::endl;
    auto codes_name = [](bool use) {
        return use ? "Comp" : "Plain";
    };
    os << "--- Stat of " << "DAFoundation N:" << codes_name(kCompressNext) << "|C:" << codes_name(kUseStrId) << " ---" << endl;
    os << "size:   " << size_in_bytes() << endl;
    os << "size bytes:   " << multiple_base_.size_in_bytes() << endl;
    os << "size next:   " << num_elements() * multiple_base_.element_size(kENNext) + next_link_bits_.size_in_bytes() +  next_flow_.size_in_bytes() << endl;
    os << "size check:   " << num_elements() + check_link_bits_.size_in_bytes() + check_flow_.size_in_bytes() << endl;
    if constexpr (kHashing) {
        size_t cWordsSize;
        if constexpr (kPlainWords) {
            size_t wordsSize;
            if constexpr (kCumulatesWords) {
                wordsSize = num_elements() / 2 + words_link_bits_.size_in_bytes() + words_flow_.size_in_bytes();
                cWordsSize = num_elements() / 2 + cum_words_link_bits_.size_in_bytes() + cum_words_flow_.size_in_bytes();
            } else {
                wordsSize = num_elements() * multiple_base_.element_size(kENWords);
                cWordsSize = num_elements() * multiple_base_.element_size(kENCWords);
            }
            os << "size words:   " << wordsSize << endl;
        } else {
            cWordsSize = kCumulatesWords ? (num_elements() + cum_words_link_bits_.size_in_bytes() + cum_words_flow_.size_in_bytes()) : (num_elements() * multiple_base_.element_size(kENCWords));
        }
        os << "size cumWords:   " << cWordsSize << endl;
        
        os << "---  ---" << endl;
    }
    //        showSizeMap(os);
}

    
template<bool CompNext, bool CompCheck, bool UnionCheckAndId, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords>
void DAFoundation<CompNext, CompCheck, UnionCheckAndId, CompId, Hashing, CompWords, CumuWords, PlainWords>::
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

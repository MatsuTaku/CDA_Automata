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

template <bool CompNext, bool UseStrId, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords>
class DAFoundation : IOInterface {
    using Self = DAFoundation<CompNext, UseStrId, CompId, Hashing, CompWords, CumuWords, PlainWords>;
    static constexpr bool kCompressNext = CompNext;
    static constexpr bool kUseStrId = UseStrId;
    static constexpr bool kCompressStrId = CompId;
    static constexpr bool kHashing = Hashing;
    static constexpr bool kCompressWords = CompWords;
    static constexpr bool kCumulatesWords = CumuWords;
    static constexpr bool kPlainWords = PlainWords;
    
    static constexpr id_type kAcceptMask = 0b01;
    static constexpr id_type kIsStrIdMask = 0b10;
    static constexpr size_t kFlags = !kCompressNext ? (kCompressStrId ? 2 : 1) : 0;
    
    static constexpr size_t kNextUpperBits = 8;
    static constexpr id_type kNextUpperMask = width_mask<kNextUpperBits>;
    static constexpr size_t kWordsUpperBits = kPlainWords ? 4 : 8;
    static constexpr id_type kWordsUpperMask = width_mask<kWordsUpperBits>;
    enum ElementNumber {
        kENNext = 0,
        kENCheck = 1,
        kENCWords = 2,
        kENWords = 3,
    };
    
    using bit_vector = sim_ds::BitVector;
    using dac_vector = sim_ds::DacVector;
    using fit_vector = sim_ds::FitVector;
    
    MultipleVector multiple_base_;
    // Enabled at NextCheck<true, C>
    bit_vector next_link_bits_;
    dac_vector next_flow_;
    // Enabled at NextCheck<N, true>
    bit_vector check_link_bits_;
    fit_vector check_flow_;
    // Enable ar NextCheck<N, C, true, true, CW>
    bit_vector words_link_bits_;
    fit_vector words_flow_;
    bit_vector cum_words_link_bits_;
    fit_vector cum_words_flow_;
    
    // For build
    using array_type = std::vector<size_t>;
    array_type next_flow_src_;
    array_type check_flow_src_;
    array_type words_flow_src_;
    array_type cum_words_flow_src_;
    
public:
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
                return ne | (next_flow_[rank] << kNextUpperBits);
            }
        }
    }
    
    uint8_t check(size_t index) const {
        return static_cast<uint8_t>(multiple_base_.nested_element<kENCheck>(index));
    }
    
    bool is_string(size_t index) const {
        assert(!kCompressNext);
        
        if constexpr (kCompressStrId) {
            return static_cast<bool>(unmasked_flags_(index) & kIsStrIdMask);
        } else {
            return check_link_bits_[index];
        }
    }
    
    bool is_final(size_t index) const {
        assert(!kCompressNext);
        
        return static_cast<bool>(unmasked_flags_(index) & kAcceptMask);
    }
    
    size_t string_id(size_t index) const {
        assert(kUseStrId);
        
        auto id = check(index);
        if (kCompressStrId && !check_link_bits_[index]) {
            return id;
        } else {
            auto rank = check_link_bits_.rank(index);
            return id | (check_flow_[rank] << kNextUpperBits);
        }
    }
    
    size_t cum_words(size_t index) const {
        assert(kHashing);
        assert(kCumulatesWords);
        
        if constexpr (kCompressWords) {
            if constexpr (kPlainWords) {
                size_t cw = multiple_base_.nested_element<kENCWords>(index) & kWordsUpperMask;
                if (!cum_words_link_bits_[index])
                    return cw;
                else {
                    auto rank = cum_words_link_bits_.rank(index);
                    return cw | (cum_words_flow_[rank] << kWordsUpperBits);
                }
            } else {
                size_t cw = multiple_base_.nested_element<kENCWords>(index);
                if (!cum_words_link_bits_[index])
                    return cw;
                else {
                    auto rank = cum_words_link_bits_.rank(index);
                    return cw | (cum_words_flow_[rank] << kWordsUpperBits);
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
            size_t words = multiple_base_.nested_element<kENCWords>(index) >> kWordsUpperBits;
            if (!words_link_bits_[index])
                return words;
            else {
                auto rank = words_link_bits_.rank(index);
                return words | (words_flow_[rank] << kWordsUpperBits);
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
            v |= kAcceptMask;
        else
            v &= ~kAcceptMask;
        multiple_base_.block(index).template restricted_set<kENNext, uint8_t>(v);
    }
    
    void set_is_string(size_t index, bool is_str) {
        assert(kUseStrId);
        if constexpr (kCompressStrId) {
            auto v = unmasked_flags_(index);
            if (is_str)
                v |= kIsStrIdMask;
            else
                v &= ~kIsStrIdMask;
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
            multiple_base_.set_nested_element<kENNext>(index, next & kNextUpperMask);
            auto flow = next >> kNextUpperBits;
            bool hasFlow = flow > 0;
            next_link_bits_[index] = hasFlow;
            if (hasFlow)
                next_flow_src_.emplace_back(flow);
        }
        
    }
    
    void set_check(size_t index, uint8_t check) {
        multiple_base_.set_nested_element<kENCheck>(index, check);
    }
    
    void set_string_id(size_t index, size_t strIndex) {
        assert(kUseStrId);
        
        set_check(index, strIndex & 0xff);
        auto flow = strIndex >> kNextUpperBits;
        bool hasFlow = flow > 0;
        if constexpr (kCompressStrId)
            check_link_bits_[index] = hasFlow;
        if (!kCompressStrId || (kCompressStrId && hasFlow))
            check_flow_src_.emplace_back(flow);
    }
    
    void set_cum_words(size_t index, size_t cw) {
        assert(kHashing);
        assert(kCumulatesWords);
        
        if constexpr (kCompressWords) {
            if constexpr (kPlainWords) {
                // Shared same byte with words to save each upper 4bits.
                uint8_t base = multiple_base_.nested_element<kENCWords>(index);
                base &= kWordsUpperMask << kWordsUpperBits;
                base |= cw & kWordsUpperMask;
                multiple_base_.set_nested_element<kENCWords>(index, base);
                auto flow = cw >> kWordsUpperBits;
                bool hasFlow = flow > 0;
                cum_words_link_bits_[index] = hasFlow;
                if (hasFlow)
                    cum_words_flow_src_.emplace_back(flow);
            } else {
                multiple_base_.set_nested_element<kENCWords>(index, cw & kWordsUpperMask);
                auto flow = cw >> kWordsUpperBits;
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
            base &= kWordsUpperMask;
            base |= ((words & kWordsUpperMask) << kWordsUpperBits);
            multiple_base_.set_nested_element<kENCWords>(index, base);
            auto flow = words >> kWordsUpperBits;
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
            check_flow_ = fit_vector(check_flow_src_);
        }
        if constexpr (kHashing) {
            if constexpr (kPlainWords) {
                words_link_bits_.Build();
                words_flow_ = fit_vector(words_flow_src_);
            }
            cum_words_link_bits_.Build();
            cum_words_flow_ = fit_vector(cum_words_flow_src_);
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
    
    void Write(std::ostream& os) const override {
        multiple_base_.Write(os);
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
    
    void Read(std::istream& is) override {
        multiple_base_.Read(is);
        if constexpr (kCompressNext) {
            next_link_bits_.Read(is);
            next_flow_.Read(is);
        }
        if constexpr (kUseStrId) {
            check_link_bits_.Read(is);
            check_flow_ = fit_vector(is);
        }
        if constexpr (kHashing && kCompressWords) {
            if constexpr (kPlainWords) {
                words_link_bits_.Read(is);
                words_flow_ = fit_vector(is);
            }
            if constexpr (kCumulatesWords) {
                cum_words_link_bits_.Read(is);
                cum_words_flow_ = fit_vector(is);
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
    uint8_t unmasked_flags_(size_t index) const {
        assert(!kCompressNext);
        return multiple_base_.block(index).template restricted_get<kENNext, uint8_t>();
    }
    
};


template<bool CompNext, bool CompCheck, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords>
void DAFoundation<CompNext, CompCheck, CompId, Hashing, CompWords, CumuWords, PlainWords>::ShowStats(std::ostream& os) const {
    using std::endl;
    auto codesName = [](bool use) {
        return use ? "DACs" : "Plain";
    };
    os << "--- Stat of " << "DAFoundation " << codesName(kCompressNext) << "|" << codesName(kUseStrId) << " ---" << endl;
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

    
template<bool CompNext, bool CompCheck, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords>
void DAFoundation<CompNext, CompCheck, CompId, Hashing, CompWords, CumuWords, PlainWords>::ShowSizeMap(std::ostream& os) const {
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

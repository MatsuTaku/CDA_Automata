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
        
        MultipleVector elements_;
        // Enabled at NextCheck<true, C>
        bit_vector nextLinkBits_;
        dac_vector nextFlow_;
        // Enabled at NextCheck<N, true>
        bit_vector checkLinkBits_;
        fit_vector checkFlow_;
        // Enable ar NextCheck<N, C, true, true, CW>
        bit_vector wordsLinkBits_;
        fit_vector wordsFlow_;
        bit_vector cumWordsLinkBits_;
        fit_vector cumWordsFlow_;
        
        // For build
        using array_type = std::vector<size_t>;
        array_type nextFlowSrc_;
        array_type checkFlowSrc_;
        array_type wordsFlowSrc_;
        array_type cumWordsFlowSrc_;
        
    public:
        DAFoundation(size_t size, size_t numQueries = 0) {
            resize(size, numQueries);
        }
        
        size_t next(size_t index) const {
            size_t next = elements_.get<kENNext>(index);
            if constexpr (!kCompressNext) {
                return next >> kFlags;
            } else {
                if (!nextLinkBits_[index]) {
                    return next;
                } else {
                    auto rank = nextLinkBits_.rank(index);
                    return next | (nextFlow_[rank] << kNextUpperBits);
                }
            }
        }
        
        uint8_t check(size_t index) const {
            return elements_.get<kENCheck, uint8_t>(index);
        }
        
        bool is_string(size_t index) const {
            assert(!kCompressNext);
            if constexpr (kCompressStrId)
                return static_cast<bool>(elements_.get<kENNext, uint8_t>(index) & kIsStrIdMask);
            else
                return checkLinkBits_[index];
        }
        
        bool is_final(size_t index) const {
            assert(!kCompressNext);
            return static_cast<bool>(elements_.get<kENNext, uint8_t>(index) & kAcceptMask);
        }
        
        size_t string_id(size_t index) const {
            assert(kUseStrId);
            
            auto id = check(index);
            if (kCompressStrId && !checkLinkBits_[index]) {
                return id;
            } else {
                auto rank = checkLinkBits_.rank(index);
                return id | (checkFlow_[rank] << kNextUpperBits);
            }
        }
        
        size_t words(size_t index) const {
            assert(kHashing);
            assert(kPlainWords);
            if constexpr (kCompressWords) {
                size_t words = elements_.get<kENCWords, uint8_t>(index) >> kWordsUpperBits;
                if (!wordsLinkBits_[index])
                    return words;
                else {
                    auto rank = wordsLinkBits_.rank(index);
                    return words | (wordsFlow_[rank] << kWordsUpperBits);
                }
            } else {
                return elements_.get<kENWords>(index);
            }
        }
        
        size_t cum_words(size_t index) const {
            assert(kHashing);
            assert(kCumulatesWords);
            
            if constexpr (kCompressWords) {
                if constexpr (kPlainWords) {
                    size_t cw = elements_.get<kENCWords, uint8_t>(index) & kWordsUpperMask;
                    if (!cumWordsLinkBits_[index])
                        return cw;
                    else {
                        auto rank = cumWordsLinkBits_.rank(index);
                        return cw | (cumWordsFlow_[rank] << kWordsUpperBits);
                    }
                } else {
                    size_t cw = elements_.get<kENCWords, uint8_t>(index);
                    if (!cumWordsLinkBits_[index])
                        return cw;
                    else {
                        auto rank = cumWordsLinkBits_.rank(index);
                        return cw | (cumWordsFlow_[rank] << kWordsUpperBits);
                    }
                }
            } else {
                return elements_.get<kENCWords>(index);
            }
            
        }
        
        size_t num_elements() const {
            return elements_.size();
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
            elements_.setValueSizes(sizes);
            elements_.resize(size);
            if (kCompressNext) nextLinkBits_.resize(size);
            if (kUseStrId) checkLinkBits_.resize(size);
            if (kHashing && kCompressWords) {
                cumWordsLinkBits_.resize(size);
                if (kPlainWords)
                    wordsLinkBits_.resize(size);
            }
        }
        
        void set_is_final(size_t index, bool isFinal) {
            assert(!kCompressNext);
            auto value = elements_.get<kENNext, uint8_t>(index);
            if (isFinal)
                value |= kAcceptMask;
            else
                value &= ~kAcceptMask;
            elements_.set<kENNext, uint8_t>(index, value);
        }
        
        void set_is_string(size_t index, bool isStr) {
            assert(kUseStrId);
            if constexpr (kCompressStrId) {
                auto value = elements_.get<kENNext, uint8_t>(index);
                if (isStr)
                    value |= kIsStrIdMask;
                else
                    value &= ~kIsStrIdMask;
                elements_.set<kENNext, uint8_t>(index, value);
            } else {
                checkLinkBits_[index] = isStr;
            }
        }
        
        void set_next(size_t index, size_t next) {
            if constexpr (!kCompressNext) {
                auto flags = elements_.get<kENNext, uint8_t>(index) & width_mask<kFlags>;
                elements_.set<kENNext>(index, next << kFlags | flags);
            } else {
                elements_.set<kENNext>(index, next & kNextUpperMask);
                auto flow = next >> kNextUpperBits;
                bool hasFlow = flow > 0;
                nextLinkBits_[index] = hasFlow;
                if (hasFlow)
                    nextFlowSrc_.emplace_back(flow);
            }
            
        }
        
        void set_check(size_t index, uint8_t check) {
            elements_.set<kENCheck>(index, check);
        }
        
        void set_string_id(size_t index, size_t strIndex) {
            assert(kUseStrId);
            
            set_check(index, strIndex & 0xff);
            auto flow = strIndex >> kNextUpperBits;
            bool hasFlow = flow > 0;
            if constexpr (kCompressStrId)
                checkLinkBits_[index] = hasFlow;
            if (!kCompressStrId || (kCompressStrId && hasFlow))
                checkFlowSrc_.emplace_back(flow);
        }
        
        void set_words(size_t index, size_t words) {
            assert(kHashing);
            assert(kPlainWords);
            
            if constexpr (kCompressWords) {
                // Shared same byte with c-store to save each upper 4bits.
                auto base = elements_.get<kENCWords, uint8_t>(index) & kWordsUpperMask;
                elements_.set<kENCWords>(index, base | ((words & kWordsUpperMask) << kWordsUpperBits));
                auto flow = words >> kWordsUpperBits;
                bool hasFlow = flow > 0;
                wordsLinkBits_[index] = hasFlow;
                if (hasFlow)
                    wordsFlowSrc_.emplace_back(flow);
            } else {
                elements_.set<kENWords>(index, words);
            }
        }
        
        void set_cum_words(size_t index, size_t cw) {
            assert(kHashing);
            assert(kCumulatesWords);
            
            if constexpr (kCompressWords) {
                if constexpr (kPlainWords) {
                    auto base = elements_.get<kENCWords, uint8_t>(index) & (kWordsUpperMask << kWordsUpperBits);
                    elements_.set<kENCWords>(index, base | (cw & kWordsUpperMask));
                    auto flow = cw >> kWordsUpperBits;
                    bool hasFlow = flow > 0;
                    cumWordsLinkBits_[index] = hasFlow;
                    if (hasFlow)
                        cumWordsFlowSrc_.emplace_back(flow);
                } else {
                    elements_.set<kENCWords>(index, cw & kWordsUpperMask);
                    auto flow = cw >> kWordsUpperBits;
                    bool hasFlow = flow > 0;
                    cumWordsLinkBits_[index] = hasFlow;
                    if (hasFlow)
                        cumWordsFlowSrc_.emplace_back(flow);
                }
            } else {
                elements_.set<kENCWords>(index, cw);
            }
        }
        
        // Finaly. Serialize flows.
        void Build() {
            if constexpr (kCompressNext) {
                nextLinkBits_.Build();
                nextFlow_.ConvertFromVector(nextFlowSrc_);
                nextFlow_.Build();
            }
            if constexpr (kUseStrId) {
                checkLinkBits_.Build();
                checkFlow_ = fit_vector(checkFlowSrc_);
            }
            if constexpr (kHashing) {
                if constexpr (kPlainWords) {
                    wordsLinkBits_.Build();
                    wordsFlow_ = fit_vector(wordsFlowSrc_);
                }
                cumWordsLinkBits_.Build();
                cumWordsFlow_ = fit_vector(cumWordsFlowSrc_);
            }
        }
        
        // MARK: - IO
        
        size_t size_in_Bytes() const override {
            auto size = elements_.size_in_Bytes();
            if constexpr (kCompressNext) {
                size += nextLinkBits_.size_in_bytes();
                size += nextFlow_.size_in_bytes();
            }
            if constexpr (kUseStrId) {
                size += checkLinkBits_.size_in_bytes();
                size += checkFlow_.size_in_bytes();
            }
            if constexpr (kHashing && kCompressWords) {
                if constexpr (kPlainWords) {
                    size += wordsLinkBits_.size_in_bytes();
                    size += wordsFlow_.size_in_bytes();
                }
                if constexpr (kCumulatesWords) {
                    size += cumWordsLinkBits_.size_in_bytes();
                    size += cumWordsFlow_.size_in_bytes();
                }
            }
            return size;
        }
        
        void Write(std::ostream& os) const override {
            elements_.Write(os);
            if constexpr (kCompressNext) {
                nextLinkBits_.Write(os);
                nextFlow_.Write(os);
            }
            if constexpr (kUseStrId) {
                checkLinkBits_.Write(os);
                checkFlow_.Write(os);
            }
            if constexpr (kHashing && kCompressWords) {
                if constexpr (kPlainWords) {
                    wordsLinkBits_.Write(os);
                    wordsFlow_.Write(os);
                }
                if constexpr (kCumulatesWords) {
                    cumWordsLinkBits_.Write(os);
                    cumWordsFlow_.Write(os);
                }
            }
        }
        
        void Read(std::istream& is) override {
            elements_.Read(is);
            if constexpr (kCompressNext) {
                nextLinkBits_.Read(is);
                nextFlow_.Read(is);
            }
            if constexpr (kUseStrId) {
                checkLinkBits_.Read(is);
                checkFlow_ = fit_vector(is);
            }
            if constexpr (kHashing && kCompressWords) {
                if constexpr (kPlainWords) {
                    wordsLinkBits_.Read(is);
                    wordsFlow_ = fit_vector(is);
                }
                if constexpr (kCumulatesWords) {
                    cumWordsLinkBits_.Read(is);
                    cumWordsFlow_ = fit_vector(is);
                }
            }
        }
        
        void ShowStatus(std::ostream& os) const override;
        
        void ShowSizeMap(std::ostream& os) const;
        
        // MARK: Copy guard
        
        DAFoundation() = default;
        ~DAFoundation() = default;
        
        DAFoundation (const DAFoundation&) = delete;
        DAFoundation& operator =(const DAFoundation&) = delete;
        
        DAFoundation (DAFoundation&&) noexcept = default;
        DAFoundation& operator =(DAFoundation&&) noexcept = default;
        
    };
    
    
    template<bool CompNext, bool CompCheck, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords>
    void DAFoundation<CompNext, CompCheck, CompId, Hashing, CompWords, CumuWords, PlainWords>::ShowStatus(std::ostream& os) const {
        using std::endl;
        auto codesName = [](bool use) {
            return use ? "DACs" : "Plain";
        };
        os << "--- Stat of " << "DAFoundation " << codesName(kCompressNext) << "|" << codesName(kUseStrId) << " ---" << endl;
        os << "size:   " << size_in_Bytes() << endl;
        os << "size bytes:   " << elements_.size_in_Bytes() << endl;
        os << "size next:   " << num_elements() * elements_.valueSize(kENNext) + nextLinkBits_.size_in_bytes() +  nextFlow_.size_in_bytes() << endl;
        os << "size check:   " << num_elements() + checkLinkBits_.size_in_bytes() + checkFlow_.size_in_bytes() << endl;
        if constexpr (kHashing) {
            size_t cWordsSize;
            if constexpr (kPlainWords) {
                size_t wordsSize;
                if constexpr (kCumulatesWords) {
                    wordsSize = num_elements() / 2 + wordsLinkBits_.size_in_bytes() + wordsFlow_.size_in_bytes();
                    cWordsSize = num_elements() / 2 + cumWordsLinkBits_.size_in_bytes() + cumWordsFlow_.size_in_bytes();
                } else {
                    wordsSize = num_elements() * elements_.valueSize(kENWords);
                    cWordsSize = num_elements() * elements_.valueSize(kENCWords);
                }
                os << "size words:   " << wordsSize << endl;
            } else {
                cWordsSize = kCumulatesWords ? (num_elements() + cumWordsLinkBits_.size_in_bytes() + cumWordsFlow_.size_in_bytes()) : (num_elements() * elements_.valueSize(kENCWords));
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

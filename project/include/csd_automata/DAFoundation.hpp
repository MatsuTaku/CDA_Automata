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
    
    template<bool CompNext, bool CompCheck, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords>
    class DAFoundation : IOInterface {
        static constexpr bool kCompressNext = CompNext;
        static constexpr bool kCompressCheck = CompCheck;
        static constexpr bool kCompressStrId = CompId;
        static constexpr bool kHashing = Hashing;
        static constexpr bool kCompressWords = CompWords;
        static constexpr bool kCumulatesWords = CumuWords;
        static constexpr bool kPlainWords = PlainWords;
        
        static constexpr id_type kAcceptMask = 0b01;
        static constexpr id_type kIsStrIdMask = 0b10;
        static constexpr size_t kFlags = !kCompressNext ? (kCompressStrId ? 2 : 1) : 0;
        
        static constexpr size_t kNextUpperBits = 8;
        static constexpr id_type kNextUpperMask = sim_ds::bit_tools::bits_mask<kNextUpperBits>;
        static constexpr size_t kWordsUpperBits = kPlainWords ? 4 : 8;
        static constexpr id_type kWordsUpperMask = sim_ds::bit_tools::bits_mask<kWordsUpperBits>;
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
        
        bool isString(size_t index) const {
            assert(!kCompressNext);
            if constexpr (kCompressStrId)
                return static_cast<bool>(elements_.get<kENNext, uint8_t>(index) & kIsStrIdMask);
            else
                return checkLinkBits_[index];
        }
        
        bool isFinal(size_t index) const {
            assert(!kCompressNext);
            return static_cast<bool>(elements_.get<kENNext, uint8_t>(index) & kAcceptMask);
        }
        
        size_t stringId(size_t index) const {
            assert(kCompressCheck);
            
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
        
        size_t cumWords(size_t index) const {
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
        
        size_t numElements() const {
            return elements_.size();
        }
        
        // MARK: - Protocol settings for build
        
        // First. Set size of elements
        void resize(size_t size, size_t words = 0) {
            auto nextSize = sim_ds::calc::sizeFitsInBytes(size << kFlags);
            std::vector<size_t> sizes = { kCompressNext ? 1 : nextSize, 1 };
            if constexpr (kHashing) {
                if constexpr (kCompressWords) {
                    sizes.push_back(1);
                } else {
                    auto wordsSize = sim_ds::calc::sizeFitsInBytes(words);
                    sizes.push_back(wordsSize);
                    if (kPlainWords)
                        sizes.push_back(wordsSize);
                }
            }
            elements_.setValueSizes(sizes);
            elements_.resize(size);
            if (kCompressNext) nextLinkBits_.resize(size);
            if (kCompressCheck) checkLinkBits_.resize(size);
            if (kHashing && kCompressWords) {
                cumWordsLinkBits_.resize(size);
                if (kPlainWords)
                    wordsLinkBits_.resize(size);
            }
        }
        
        void setIsFinal(size_t index, bool isFinal) {
            assert(!kCompressNext);
            auto value = elements_.get<kENNext, uint8_t>(index);
            if (isFinal)
                value |= kAcceptMask;
            else
                value &= ~kAcceptMask;
            elements_.set<kENNext, uint8_t>(index, value);
        }
        
        void setIsString(size_t index, bool isStr) {
            assert(kCompressCheck);
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
        
        void setNext(size_t index, size_t next) {
            if constexpr (!kCompressNext) {
                auto flags = elements_.get<kENNext, uint8_t>(index) & sim_ds::bit_tools::bits_mask<kFlags>;
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
        
        void setCheck(size_t index, uint8_t check) {
            elements_.set<kENCheck>(index, check);
        }
        
        void setStringId(size_t index, size_t strIndex) {
            assert(kCompressCheck);
            
            setCheck(index, strIndex & 0xff);
            auto flow = strIndex >> kNextUpperBits;
            bool hasFlow = flow > 0;
            if constexpr (kCompressStrId)
                checkLinkBits_[index] = hasFlow;
            if (!kCompressStrId || (kCompressStrId && hasFlow))
                checkFlowSrc_.emplace_back(flow);
        }
        
        void setWords(size_t index, size_t words) {
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
        
        void setCumWords(size_t index, size_t cw) {
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
        void build() {
            if constexpr (kCompressNext) {
                nextLinkBits_.build();
                nextFlow_.setVector(nextFlowSrc_);
                nextFlow_.build();
            }
            if constexpr (kCompressCheck) {
                checkLinkBits_.build();
                checkFlow_ = fit_vector(checkFlowSrc_);
            }
            if constexpr (kHashing) {
                if constexpr (kPlainWords) {
                    wordsLinkBits_.build();
                    wordsFlow_ = fit_vector(wordsFlowSrc_);
                }
                cumWordsLinkBits_.build();
                cumWordsFlow_ = fit_vector(cumWordsFlowSrc_);
            }
        }
        
        // MARK: - IO
        
        size_t sizeInBytes() const override {
            auto size = elements_.sizeInBytes();
            if constexpr (kCompressNext) {
                size += nextLinkBits_.sizeInBytes();
                size += nextFlow_.sizeInBytes();
            }
            if constexpr (kCompressCheck) {
                size += checkLinkBits_.sizeInBytes();
                size += checkFlow_.sizeInBytes();
            }
            if constexpr (kHashing && kCompressWords) {
                if constexpr (kPlainWords) {
                    size += wordsLinkBits_.sizeInBytes();
                    size += wordsFlow_.sizeInBytes();
                }
                if constexpr (kCumulatesWords) {
                    size += cumWordsLinkBits_.sizeInBytes();
                    size += cumWordsFlow_.sizeInBytes();
                }
            }
            return size;
        }
        
        void write(std::ostream& os) const override {
            elements_.write(os);
            if constexpr (kCompressNext) {
                nextLinkBits_.write(os);
                nextFlow_.write(os);
            }
            if constexpr (kCompressCheck) {
                checkLinkBits_.write(os);
                checkFlow_.write(os);
            }
            if constexpr (kHashing && kCompressWords) {
                if constexpr (kPlainWords) {
                    wordsLinkBits_.write(os);
                    wordsFlow_.write(os);
                }
                if constexpr (kCumulatesWords) {
                    cumWordsLinkBits_.write(os);
                    cumWordsFlow_.write(os);
                }
            }
        }
        
        void read(std::istream& is) override {
            elements_.read(is);
            if constexpr (kCompressNext) {
                nextLinkBits_.read(is);
                nextFlow_.read(is);
            }
            if constexpr (kCompressCheck) {
                checkLinkBits_.read(is);
                checkFlow_ = fit_vector(is);
            }
            if constexpr (kHashing && kCompressWords) {
                if constexpr (kPlainWords) {
                    wordsLinkBits_.read(is);
                    wordsFlow_ = fit_vector(is);
                }
                if constexpr (kCumulatesWords) {
                    cumWordsLinkBits_.read(is);
                    cumWordsFlow_ = fit_vector(is);
                }
            }
        }
        
        void showStatus(std::ostream& os) const override;
        
        void showSizeMap(std::ostream& os) const;
        
        // MARK: Copy guard
        
        DAFoundation() = default;
        ~DAFoundation() = default;
        
        DAFoundation (const DAFoundation&) = delete;
        DAFoundation& operator =(const DAFoundation&) = delete;
        
        DAFoundation (DAFoundation&&) noexcept = default;
        DAFoundation& operator =(DAFoundation&&) noexcept = default;
        
    };
    
    
    template<bool CompNext, bool CompCheck, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords>
    void DAFoundation<CompNext, CompCheck, CompId, Hashing, CompWords, CumuWords, PlainWords>::showStatus(std::ostream& os) const {
        using std::endl;
        auto codesName = [](bool use) {
            return use ? "DACs" : "Plain";
        };
        os << "--- Stat of " << "DAFoundation " << codesName(kCompressNext) << "|" << codesName(kCompressCheck) << " ---" << endl;
        os << "size:   " << sizeInBytes() << endl;
        os << "size bytes:   " << elements_.sizeInBytes() << endl;
        os << "size next:   " << numElements() * elements_.valueSize(kENNext) + nextLinkBits_.sizeInBytes() +  nextFlow_.sizeInBytes() << endl;
        os << "size check:   " << numElements() + checkLinkBits_.sizeInBytes() + checkFlow_.sizeInBytes() << endl;
        if constexpr (kHashing) {
            size_t cWordsSize;
            if constexpr (kPlainWords) {
                size_t wordsSize;
                if constexpr (kCumulatesWords) {
                    wordsSize = numElements() / 2 + wordsLinkBits_.sizeInBytes() + wordsFlow_.sizeInBytes();
                    cWordsSize = numElements() / 2 + cumWordsLinkBits_.sizeInBytes() + cumWordsFlow_.sizeInBytes();
                } else {
                    wordsSize = numElements() * elements_.valueSize(kENWords);
                    cWordsSize = numElements() * elements_.valueSize(kENCWords);
                }
                os << "size words:   " << wordsSize << endl;
            } else {
                cWordsSize = kCumulatesWords ? (numElements() + cumWordsLinkBits_.sizeInBytes() + cumWordsFlow_.sizeInBytes()) : (numElements() * elements_.valueSize(kENCWords));
            }
            os << "size cumWords:   " << cWordsSize << endl;
            
            os << "---  ---" << endl;
        }
        //        showSizeMap(os);
    }
    
    template<bool CompNext, bool CompCheck, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords>
    void DAFoundation<CompNext, CompCheck, CompId, Hashing, CompWords, CumuWords, PlainWords>::showSizeMap(std::ostream& os) const {
        auto numElem = numElements();
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

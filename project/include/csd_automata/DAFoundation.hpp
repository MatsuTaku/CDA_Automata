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

#include "sim_ds/DacVector.hpp"
#include "sim_ds/calc.hpp"

namespace csd_automata {
    
    template<bool COMP_NEXT, bool COMP_CHECK, bool COMP_ID, bool FOR_DICT, bool COMP_WORDS, bool USE_CUMU_WORDS, bool NEEDS_ACCESS>
    class DAFoundation : IOInterface {
    public:
        static constexpr bool useNextCodes = COMP_NEXT;
        static constexpr bool useCheckCodes = COMP_CHECK;
        
    private:
        static constexpr size_t kFixedBits = 8;
        static constexpr size_t kFixedBitsWords = NEEDS_ACCESS ? 4 : 0;
        enum ElementNumber {
            ENNext = 0,
            ENCheck = 1,
            ENCWords = 2,
            ENWords = 3,
        };
        
    public:
        DAFoundation() = default;
        ~DAFoundation() = default;
        
        // MARK: - Copy guard
        
        DAFoundation (const DAFoundation&) = delete;
        DAFoundation& operator =(const DAFoundation&) = delete;
        
        DAFoundation (DAFoundation&&) noexcept = default;
        DAFoundation& operator =(DAFoundation&&) noexcept = default;
        
    public:
        size_t next(size_t index) const {
            auto next = elements_.get<ENNext, size_t>(index);
            if constexpr (!COMP_NEXT) {
                return next >> (COMP_ID ? 2 : 1);
            }
            
            if (!nextLinkBits_[index])
                return next;
            else {
                auto rank = nextLinkBits_.rank(index);
                return next | (nextFlow_[rank] << kFixedBits);
            }
        }
        
        uint8_t check(size_t index) const {
            return elements_.get<ENCheck, uint8_t>(index);
        }
        
        bool isString(size_t index) const {
            assert(!useNextCodes);
            if constexpr (COMP_ID)
                return (elements_.get<ENNext, uint8_t>(index) & 2) != 0;
            else
                return checkLinkBits_[index];
        }
        
        bool isFinal(size_t index) const {
            assert(!useNextCodes);
            return (elements_.get<ENNext, uint8_t>(index) & 1) != 0;
        }
        
        size_t stringId(size_t index) const {
            assert(useCheckCodes);
            
            auto id = check(index);
            if (COMP_ID && !checkLinkBits_[index]) {
                return id;
            } else {
                auto rank = checkLinkBits_.rank(index);
                return id | (checkFlow_[rank] << kFixedBits);
            }
        }
        
        size_t words(size_t index) const {
            assert(FOR_DICT);
            assert(NEEDS_ACCESS);
            if constexpr (COMP_WORDS) {
                size_t words = elements_.get<ENCWords, uint8_t>(index) >> kFixedBitsWords;
                if (!wordsLinkBits_[index])
                    return words;
                else {
                    auto rank = wordsLinkBits_.rank(index);
                    return words | (wordsFlow_[rank] << kFixedBitsWords);
                }
            } else {
                return elements_.get<ENWords, size_t>(index);
            }
        }
        
        size_t cumWords(size_t index) const {
            assert(FOR_DICT);
            assert(USE_CUMU_WORDS);
            
            if constexpr (COMP_WORDS) {
                if constexpr (NEEDS_ACCESS) {
                    size_t cw = elements_.get<ENCWords, uint8_t>(index) & 0x0F;
                    if (!cumWordsLinkBits_[index])
                        return cw;
                    else {
                        auto rank = cumWordsLinkBits_.rank(index);
                        return cw | (cumWordsFlow_[rank] << kFixedBitsWords);
                    }
                } else {
                    size_t cw = elements_.get<ENCWords, uint8_t>(index);
                    if (!cumWordsLinkBits_[index])
                        return cw;
                    else {
                        auto rank = cumWordsLinkBits_.rank(index);
                        return cw | (cumWordsFlow_[rank] << kFixedBits);
                    }
                }
            } else {
                return elements_.get<ENCWords, size_t>(index);
            }
            
        }
        
        size_t numElements() const {
            return elements_.size();
        }
        
        // MARK: - build
        
        void setIsFinal(size_t index, bool isFinal) {
            assert(!useNextCodes);
            auto value = elements_.get<ENNext, uint8_t>(index);
            value = (value & ~(1)) | isFinal;
            elements_.set<ENNext>(index, value);
        }
        
        void setIsString(size_t index, bool isStr) {
            assert(!useNextCodes);
            if constexpr (COMP_ID) {
                auto value = elements_.get<ENNext, uint8_t>(index);
                value = (value & ~(1 << 1)) | (isStr << 1);
                elements_.set<ENNext>(index, value);
            } else {
                checkLinkBits_.set(index, isStr);
            }
        }
        
        void setNext(size_t index, size_t next) {
            if constexpr (!COMP_NEXT)
                next <<= (COMP_ID ? 2 : 1);
            elements_.set<ENNext>(index, next);
            
            if constexpr (!COMP_NEXT) return;
            auto flow = next >> kFixedBits;
            bool hasFlow = flow > 0;
            nextLinkBits_.set(index, hasFlow);
            if (hasFlow)
                nextFlowSrc_.emplace_back(flow);
        }
        
        void setCheck(size_t index, uint8_t check) {
            elements_.set<ENCheck>(index, check);
        }
        
        void setStringId(size_t index, size_t strIndex) {
            assert(useCheckCodes);
            
            setCheck(index, strIndex & 0xff);
            auto flow = strIndex >> kFixedBits;
            bool hasFlow = flow > 0;
            if constexpr (COMP_ID)
                checkLinkBits_.set(index, hasFlow);
            if (!COMP_ID || hasFlow)
                checkFlowSrc_.emplace_back(flow);
        }
        
        void setWords(size_t index, size_t words) {
            assert(FOR_DICT);
            assert(NEEDS_ACCESS);
            
            if constexpr (COMP_WORDS) {
                // Shared same byte with c-store to save each upper 4bits.
                auto base = elements_.get<ENCWords, uint8_t>(index) & 0x0F;
                elements_.set<ENCWords>(index, base | ((words & 0x0F) << kFixedBitsWords));
                auto flow = words >> kFixedBitsWords;
                bool hasFlow = flow > 0;
                wordsLinkBits_.set(index, hasFlow);
                if (hasFlow)
                    wordsFlowSrc_.emplace_back(flow);
            } else {
                elements_.set<ENWords>(index, words);
            }
        }
        
        void setCumWords(size_t index, size_t cw) {
            assert(FOR_DICT);
            assert(USE_CUMU_WORDS);
            
            if constexpr (COMP_WORDS) {
                if constexpr (NEEDS_ACCESS) {
                    auto base = elements_.get<ENCWords, uint8_t>(index) & 0xF0;
                    elements_.set<ENCWords>(index, base | (cw & 0x0F));
                    auto flow = cw >> kFixedBitsWords;
                    bool hasFlow = flow > 0;
                    cumWordsLinkBits_.set(index, hasFlow);
                    if (hasFlow)
                        cumWordsFlowSrc_.emplace_back(flow);
                } else {
                    elements_.set<ENCWords>(index, cw & 0xFF);
                    auto flow = cw >> kFixedBits;
                    bool hasFlow = flow > 0;
                    cumWordsLinkBits_.set(index, hasFlow);
                    if (hasFlow)
                        cumWordsFlowSrc_.emplace_back(flow);
                }
            } else {
                elements_.set<ENCWords>(index, cw);
            }
        }
        
        // MARK: - Protocol settings
        
        // First. Set size of elements
        void resize(size_t size, size_t words = 0) {
            auto bitInto = !useNextCodes;
            auto nextSize = sim_ds::calc::sizeFitInBytes(bitInto ? (size << 2) : size);
            std::vector<size_t> sizes = { COMP_NEXT ? 1 : nextSize, 1 };
            if constexpr (FOR_DICT) {
                if constexpr (COMP_WORDS) {
                    sizes.push_back(1);
                } else {
                    auto wordsSize = sim_ds::calc::sizeFitInBytes(words);
                    sizes.push_back(wordsSize);
                    if (NEEDS_ACCESS)
                        sizes.push_back(wordsSize);
                }
            }
            elements_.setValueSizes(sizes);
            elements_.resize(size);
            if (COMP_NEXT) nextLinkBits_.resize(size);
            if (COMP_CHECK) checkLinkBits_.resize(size);
            if (FOR_DICT && COMP_WORDS) {
                cumWordsLinkBits_.resize(size);
                if (NEEDS_ACCESS)
                    wordsLinkBits_.resize(size);
            }
        }
        
        // Finaly. If use dac
        void build() {
            if constexpr (COMP_NEXT) {
                nextLinkBits_.build();
                nextFlow_.setVector(nextFlowSrc_);
                nextFlow_.build();
            }
            if constexpr (COMP_CHECK) {
                checkLinkBits_.build();
                checkFlow_ = sim_ds::FitVector(checkFlowSrc_);
            }
            if constexpr (FOR_DICT) {
                if constexpr (NEEDS_ACCESS) {
                    wordsLinkBits_.build();
                    wordsFlow_ = sim_ds::FitVector(wordsFlowSrc_);
                }
                cumWordsLinkBits_.build();
                cumWordsFlow_ = sim_ds::FitVector(cumWordsFlowSrc_);
            }
        }
        
        // MARK: - ByteData methods
        
        size_t sizeInBytes() const override {
            auto size = elements_.sizeInBytes();
            if constexpr (COMP_NEXT) {
                size += nextLinkBits_.sizeInBytes();
                size += nextFlow_.sizeInBytes();
            }
            if constexpr (COMP_CHECK) {
                size += checkLinkBits_.sizeInBytes();
                size += checkFlow_.sizeInBytes();
            }
            if constexpr (FOR_DICT && COMP_WORDS) {
                if constexpr (NEEDS_ACCESS) {
                    size += wordsLinkBits_.sizeInBytes();
                    size += wordsFlow_.sizeInBytes();
                }
                if constexpr (USE_CUMU_WORDS) {
                    size += cumWordsLinkBits_.sizeInBytes();
                    size += cumWordsFlow_.sizeInBytes();
                }
            }
            return size;
        }
        
        void write(std::ostream &os) const override {
            elements_.write(os);
            if constexpr (COMP_NEXT) {
                nextLinkBits_.write(os);
                nextFlow_.write(os);
            }
            if constexpr (COMP_CHECK) {
                checkLinkBits_.write(os);
                checkFlow_.write(os);
            }
            if constexpr (FOR_DICT && COMP_WORDS) {
                if constexpr (NEEDS_ACCESS) {
                    wordsLinkBits_.write(os);
                    wordsFlow_.write(os);
                }
                if constexpr (USE_CUMU_WORDS) {
                    cumWordsLinkBits_.write(os);
                    cumWordsFlow_.write(os);
                }
            }
        }
        
        void read(std::istream &is) override {
            elements_.read(is);
            if constexpr (COMP_NEXT) {
                nextLinkBits_.read(is);
                nextFlow_.read(is);
            }
            if constexpr (COMP_CHECK) {
                checkLinkBits_.read(is);
                checkFlow_ = sim_ds::FitVector(is);
            }
            if constexpr (FOR_DICT && COMP_WORDS) {
                if constexpr (NEEDS_ACCESS) {
                    wordsLinkBits_.read(is);
                    wordsFlow_ = sim_ds::FitVector(is);
                }
                if constexpr (USE_CUMU_WORDS) {
                    cumWordsLinkBits_.read(is);
                    cumWordsFlow_ = sim_ds::FitVector(is);
                }
            }
        }
        
        void showStatus(std::ostream &os) const override;
        void showSizeMap(std::ostream &os) const;
        
    private:
        MultipleVector elements_;
        // Enabled at NextCheck<true, C>
        sim_ds::BitVector nextLinkBits_;
        sim_ds::DacVector nextFlow_;
        // Enabled at NextCheck<N, true>
        sim_ds::BitVector checkLinkBits_;
        sim_ds::FitVector checkFlow_;
        // Enable ar NextCheck<N, C, true, true, CW>
        sim_ds::BitVector wordsLinkBits_;
        sim_ds::FitVector wordsFlow_;
        sim_ds::BitVector cumWordsLinkBits_;
        sim_ds::FitVector cumWordsFlow_;
        
        // For build
        std::vector<size_t> nextFlowSrc_;
        std::vector<size_t> checkFlowSrc_;
        std::vector<size_t> wordsFlowSrc_;
        std::vector<size_t> cumWordsFlowSrc_;
        
    };
    
    
    template<bool N, bool C, bool CI, bool D, bool W, bool CW, bool NA>
    void DAFoundation<N, C, CI, D, W, CW, NA>::showStatus(std::ostream &os) const {
        using std::endl;
        auto codesName = [=](bool use) {
            return use ? "DACs" : "Plain";
        };
        os << "--- Stat of " << "DAFoundation " << codesName(useNextCodes) << "|" << codesName(useCheckCodes) << " ---" << endl;
        os << "size:   " << sizeInBytes() << endl;
        os << "size bytes:   " << elements_.sizeInBytes() << endl;
        os << "size next:   " << numElements() * elements_.valueSize(ENNext) + nextLinkBits_.sizeInBytes() +  nextFlow_.sizeInBytes() << endl;
        os << "size check:   " << numElements() + checkLinkBits_.sizeInBytes() + checkFlow_.sizeInBytes() << endl;
        if constexpr (D) {
            size_t cWordsSize;
            if constexpr (NA) {
                size_t wordsSize;
                if constexpr (CW) {
                    wordsSize = numElements() / 2 + wordsLinkBits_.sizeInBytes() + wordsFlow_.sizeInBytes();
                    cWordsSize = numElements() / 2 + cumWordsLinkBits_.sizeInBytes() + cumWordsFlow_.sizeInBytes();
                } else {
                    wordsSize = numElements() * elements_.valueSize(ENWords);
                    cWordsSize = numElements() * elements_.valueSize(ENCWords);
                }
                os << "size words:   " << wordsSize << endl;
            } else {
                cWordsSize = CW ? (numElements() + cumWordsLinkBits_.sizeInBytes() + cumWordsFlow_.sizeInBytes()) : (numElements() * elements_.valueSize(ENCWords));
            }
            os << "size cumWords:   " << cWordsSize << endl;
            
            os << "---  ---" << endl;
        }
        //        showSizeMap(os);
    }
    
    template<bool N, bool C, bool CI, bool D, bool W, bool CW, bool NA>
    void DAFoundation<N, C, CI, D, W, CW, NA>::showSizeMap(std::ostream &os) const {
        auto numElem = numElements();
        std::vector<size_t> nexts(numElem);
        for (auto i = 0; i < numElem; i++)
            nexts[i] = next(i) >> (!N ? 1 : 0);
        
        auto showList = [&](const std::vector<size_t> &list) {
            using std::endl;
            os << "-- " << "Next Map" << " --" << endl;
            for (auto c : list)
                os << c << "\t" << endl;
            os << "/ " << numElem << endl;
        };
        
    }
    
}

#endif /* NextCheck_hpp */

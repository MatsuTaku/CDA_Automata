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

#include "sim_ds/calc.hpp"

namespace csd_automata {
    
    template<bool _CompNext, bool _CompCheck, bool _CompId, bool _Hashing, bool _CompWords, bool _CumuWords, bool _PlainWords>
    class DAFoundation : IOInterface {
    public:
        static constexpr bool _compress_next = _CompNext;
        static constexpr bool _compress_check = _CompCheck;
        static constexpr bool _compress_str_id = _CompId;
        static constexpr bool _hashing = _Hashing;
        static constexpr bool _compress_words = _CompWords;
        static constexpr bool _cumulates_words = _CumuWords;
        static constexpr bool _plain_words = _PlainWords;
        
        static constexpr id_type _accept_mask = 1U;
        static constexpr id_type _is_str_id_mask = 2U;
        static constexpr size_t _flags = !_compress_next ? (_compress_str_id ? 2 : 1) : 0;
        
    private:
        static constexpr size_t _next_upper_bits_ = 8;
        static constexpr id_type _next_upper_mask_ = (1ULL << _next_upper_bits_) - 1;
        static constexpr size_t _words_upper_bits_ = _plain_words ? 4 : 0;
        enum ElementNumber {
            ENNext = 0,
            ENCheck = 1,
            ENCWords = 2,
            ENWords = 3,
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
        std::vector<size_t> nextFlowSrc_;
        std::vector<size_t> checkFlowSrc_;
        std::vector<size_t> wordsFlowSrc_;
        std::vector<size_t> cumWordsFlowSrc_;
        
    public:
        DAFoundation(size_t size, size_t numQueries = 0) {
            resize(size, numQueries);
        }
        
        size_t next(size_t index) const {
            size_t next = elements_.get<ENNext>(index);
            if constexpr (!_compress_next) {
                return next >> _flags;
            } else {
                if (!nextLinkBits_[index]) {
                    return next;
                } else {
                    auto rank = nextLinkBits_.rank(index);
                    return next | (nextFlow_[rank] << _next_upper_bits_);
                }
            }
        }
        
        uint8_t check(size_t index) const {
            return elements_.get<ENCheck, uint8_t>(index);
        }
        
        bool isString(size_t index) const {
            assert(!_compress_next);
            if constexpr (_compress_str_id)
                return static_cast<bool>(elements_.get<ENNext, uint8_t>(index) & _is_str_id_mask);
            else
                return checkLinkBits_[index];
        }
        
        bool isFinal(size_t index) const {
            assert(!_compress_next);
            return static_cast<bool>(elements_.get<ENNext, uint8_t>(index) & _accept_mask);
        }
        
        size_t stringId(size_t index) const {
            assert(_compress_check);
            
            auto id = check(index);
            if (_compress_str_id && !checkLinkBits_[index]) {
                return id;
            } else {
                auto rank = checkLinkBits_.rank(index);
                return id | (checkFlow_[rank] << _next_upper_bits_);
            }
        }
        
        size_t words(size_t index) const {
            assert(_hashing);
            assert(_plain_words);
            if constexpr (_compress_words) {
                size_t words = elements_.get<ENCWords, uint8_t>(index) >> _words_upper_bits_;
                if (!wordsLinkBits_[index])
                    return words;
                else {
                    auto rank = wordsLinkBits_.rank(index);
                    return words | (wordsFlow_[rank] << _words_upper_bits_);
                }
            } else {
                return elements_.get<ENWords>(index);
            }
        }
        
        size_t cumWords(size_t index) const {
            assert(_hashing);
            assert(_cumulates_words);
            
            if constexpr (_compress_words) {
                if constexpr (_plain_words) {
                    size_t cw = elements_.get<ENCWords, uint8_t>(index) & 0x0F;
                    if (!cumWordsLinkBits_[index])
                        return cw;
                    else {
                        auto rank = cumWordsLinkBits_.rank(index);
                        return cw | (cumWordsFlow_[rank] << _words_upper_bits_);
                    }
                } else {
                    size_t cw = elements_.get<ENCWords, uint8_t>(index);
                    if (!cumWordsLinkBits_[index])
                        return cw;
                    else {
                        auto rank = cumWordsLinkBits_.rank(index);
                        return cw | (cumWordsFlow_[rank] << _next_upper_bits_);
                    }
                }
            } else {
                return elements_.get<ENCWords>(index);
            }
            
        }
        
        size_t numElements() const {
            return elements_.size();
        }
        
        // MARK: - Protocol settings for build
        
        // First. Set size of elements
        void resize(size_t size, size_t words = 0) {
            auto nextSize = sim_ds::calc::sizeFitsInBytes(size << _flags);
            std::vector<size_t> sizes = { _compress_next ? 1 : nextSize, 1 };
            if constexpr (_hashing) {
                if constexpr (_compress_words) {
                    sizes.push_back(1);
                } else {
                    auto wordsSize = sim_ds::calc::sizeFitsInBytes(words);
                    sizes.push_back(wordsSize);
                    if (_plain_words)
                        sizes.push_back(wordsSize);
                }
            }
            elements_.setValueSizes(sizes);
            elements_.resize(size);
            if (_compress_next) nextLinkBits_.resize(size);
            if (_compress_check) checkLinkBits_.resize(size);
            if (_hashing && _compress_words) {
                cumWordsLinkBits_.resize(size);
                if (_plain_words)
                    wordsLinkBits_.resize(size);
            }
        }
        
        void setIsFinal(size_t index, bool isFinal) {
            assert(!_compress_next);
            auto value = elements_.get<ENNext, uint8_t>(index);
            if (isFinal)
                value |= _accept_mask;
            else
                value &= ~_accept_mask;
            elements_.set<ENNext>(index, value);
        }
        
        void setIsString(size_t index, bool isStr) {
            assert(_compress_check);
            if constexpr (_compress_str_id) {
                auto value = elements_.get<ENNext, uint8_t>(index);
                if (isStr)
                    value |= _is_str_id_mask;
                else
                    value &= ~_is_str_id_mask;
                elements_.set<ENNext>(index, value);
            } else {
                checkLinkBits_[index] = isStr;
            }
        }
        
        void setNext(size_t index, size_t next) {
            if constexpr (!_compress_next) {
                elements_.set<ENNext>(index, next << _flags);
            } else {
                elements_.set<ENNext>(index, next & _next_upper_mask_);
                auto flow = next >> _next_upper_bits_;
                bool hasFlow = flow > 0;
                nextLinkBits_[index] = hasFlow;
                if (hasFlow)
                    nextFlowSrc_.emplace_back(flow);
            }
            
        }
        
        void setCheck(size_t index, uint8_t check) {
            elements_.set<ENCheck>(index, check);
        }
        
        void setStringId(size_t index, size_t strIndex) {
            assert(_compress_check);
            
            setCheck(index, strIndex & 0xff);
            auto flow = strIndex >> _next_upper_bits_;
            bool hasFlow = flow > 0;
            if constexpr (_compress_str_id)
                checkLinkBits_[index] = hasFlow;
            if (!_compress_str_id || (_compress_str_id && hasFlow))
                checkFlowSrc_.emplace_back(flow);
        }
        
        void setWords(size_t index, size_t words) {
            assert(_hashing);
            assert(_plain_words);
            
            if constexpr (_compress_words) {
                // Shared same byte with c-store to save each upper 4bits.
                auto base = elements_.get<ENCWords, uint8_t>(index) & 0x0F;
                elements_.set<ENCWords>(index, base | ((words & 0x0F) << _words_upper_bits_));
                auto flow = words >> _words_upper_bits_;
                bool hasFlow = flow > 0;
                wordsLinkBits_[index] = hasFlow;
                if (hasFlow)
                    wordsFlowSrc_.emplace_back(flow);
            } else {
                elements_.set<ENWords>(index, words);
            }
        }
        
        void setCumWords(size_t index, size_t cw) {
            assert(_hashing);
            assert(_cumulates_words);
            
            if constexpr (_compress_words) {
                if constexpr (_plain_words) {
                    auto base = elements_.get<ENCWords, uint8_t>(index) & 0xF0;
                    elements_.set<ENCWords>(index, base | (cw & 0x0F));
                    auto flow = cw >> _words_upper_bits_;
                    bool hasFlow = flow > 0;
                    cumWordsLinkBits_[index] = hasFlow;
                    if (hasFlow)
                        cumWordsFlowSrc_.emplace_back(flow);
                } else {
                    elements_.set<ENCWords>(index, cw & 0xFF);
                    auto flow = cw >> _next_upper_bits_;
                    bool hasFlow = flow > 0;
                    cumWordsLinkBits_[index] = hasFlow;
                    if (hasFlow)
                        cumWordsFlowSrc_.emplace_back(flow);
                }
            } else {
                elements_.set<ENCWords>(index, cw);
            }
        }
        
        // Finaly. Serialize flows.
        void build() {
            if constexpr (_compress_next) {
                nextLinkBits_.build();
                nextFlow_.setVector(nextFlowSrc_);
                nextFlow_.build();
            }
            if constexpr (_compress_check) {
                checkLinkBits_.build();
                checkFlow_ = fit_vector(checkFlowSrc_);
            }
            if constexpr (_hashing) {
                if constexpr (_plain_words) {
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
            if constexpr (_compress_next) {
                size += nextLinkBits_.sizeInBytes();
                size += nextFlow_.sizeInBytes();
            }
            if constexpr (_compress_check) {
                size += checkLinkBits_.sizeInBytes();
                size += checkFlow_.sizeInBytes();
            }
            if constexpr (_hashing && _compress_words) {
                if constexpr (_plain_words) {
                    size += wordsLinkBits_.sizeInBytes();
                    size += wordsFlow_.sizeInBytes();
                }
                if constexpr (_cumulates_words) {
                    size += cumWordsLinkBits_.sizeInBytes();
                    size += cumWordsFlow_.sizeInBytes();
                }
            }
            return size;
        }
        
        void write(std::ostream& os) const override {
            elements_.write(os);
            if constexpr (_compress_next) {
                nextLinkBits_.write(os);
                nextFlow_.write(os);
            }
            if constexpr (_compress_check) {
                checkLinkBits_.write(os);
                checkFlow_.write(os);
            }
            if constexpr (_hashing && _compress_words) {
                if constexpr (_plain_words) {
                    wordsLinkBits_.write(os);
                    wordsFlow_.write(os);
                }
                if constexpr (_cumulates_words) {
                    cumWordsLinkBits_.write(os);
                    cumWordsFlow_.write(os);
                }
            }
        }
        
        void read(std::istream& is) override {
            elements_.read(is);
            if constexpr (_compress_next) {
                nextLinkBits_.read(is);
                nextFlow_.read(is);
            }
            if constexpr (_compress_check) {
                checkLinkBits_.read(is);
                checkFlow_ = fit_vector(is);
            }
            if constexpr (_hashing && _compress_words) {
                if constexpr (_plain_words) {
                    wordsLinkBits_.read(is);
                    wordsFlow_ = fit_vector(is);
                }
                if constexpr (_cumulates_words) {
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
    
    
    template<bool _CompNext, bool _CompCheck, bool _CompId, bool _Hashing, bool _CompWords, bool _CumuWords, bool _PlainWords>
    void DAFoundation<_CompNext, _CompCheck, _CompId, _Hashing, _CompWords, _CumuWords, _PlainWords>::showStatus(std::ostream& os) const {
        using std::endl;
        auto codesName = [](bool use) {
            return use ? "DACs" : "Plain";
        };
        os << "--- Stat of " << "DAFoundation " << codesName(_compress_next) << "|" << codesName(_compress_check) << " ---" << endl;
        os << "size:   " << sizeInBytes() << endl;
        os << "size bytes:   " << elements_.sizeInBytes() << endl;
        os << "size next:   " << numElements() * elements_.valueSize(ENNext) + nextLinkBits_.sizeInBytes() +  nextFlow_.sizeInBytes() << endl;
        os << "size check:   " << numElements() + checkLinkBits_.sizeInBytes() + checkFlow_.sizeInBytes() << endl;
        if constexpr (_hashing) {
            size_t cWordsSize;
            if constexpr (_plain_words) {
                size_t wordsSize;
                if constexpr (_cumulates_words) {
                    wordsSize = numElements() / 2 + wordsLinkBits_.sizeInBytes() + wordsFlow_.sizeInBytes();
                    cWordsSize = numElements() / 2 + cumWordsLinkBits_.sizeInBytes() + cumWordsFlow_.sizeInBytes();
                } else {
                    wordsSize = numElements() * elements_.valueSize(ENWords);
                    cWordsSize = numElements() * elements_.valueSize(ENCWords);
                }
                os << "size words:   " << wordsSize << endl;
            } else {
                cWordsSize = _cumulates_words ? (numElements() + cumWordsLinkBits_.sizeInBytes() + cumWordsFlow_.sizeInBytes()) : (numElements() * elements_.valueSize(ENCWords));
            }
            os << "size cumWords:   " << cWordsSize << endl;
            
            os << "---  ---" << endl;
        }
        //        showSizeMap(os);
    }
    
    template<bool _CompNext, bool _CompCheck, bool _CompId, bool _Hashing, bool _CompWords, bool _CumuWords, bool _PlainWords>
    void DAFoundation<_CompNext, _CompCheck, _CompId, _Hashing, _CompWords, _CumuWords, _PlainWords>::showSizeMap(std::ostream& os) const {
        auto numElem = numElements();
        std::vector<size_t> nexts(numElem);
        for (auto i = 0; i < numElem; i++)
            nexts[i] = next(i) >> (!_compress_next ? 1 : 0);
        
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

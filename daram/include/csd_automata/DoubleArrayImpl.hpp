//
//  DoubleArrayImpl.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/13.
//

#ifndef Double_Array_Impr_hpp
#define Double_Array_Impr_hpp

#include "IOInterface.hpp"
#include "sim_ds/MultipleVector.hpp"
#include "sim_ds/BitVector.hpp"
#include "sim_ds/SuccinctBitVector.hpp"
#include "sim_ds/DacVector.hpp"
#include "sim_ds/FitVector.hpp"

#include "sim_ds/bit_util.hpp"
#include "sim_ds/calc.hpp"

namespace csd_automata {
    

template <bool CompNext, bool UseStrId, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords, bool LinkChildren, bool SelectId, bool DacWords>
class DoubleArrayImpl : sim_ds::MultipleVector, IOInterface {
public:
    using Self = DoubleArrayImpl<CompNext, UseStrId, CompId, Hashing, CompWords, CumuWords, PlainWords, LinkChildren, SelectId, DacWords>;
    using Base = sim_ds::MultipleVector;
    
    static constexpr bool kCompressNext = CompNext;
    static constexpr bool kUseStrId = UseStrId;
    static constexpr bool kCompressStrId = CompId;
    static constexpr bool kHashing = Hashing;
    static constexpr bool kCompressWords = CompWords;
    static constexpr bool kCumulativeWords = CumuWords;
    static constexpr bool kPlainWords = PlainWords;
    static constexpr bool kLinkChildren = LinkChildren;
    static constexpr bool kSelectId = SelectId;
    static constexpr bool kDacWords = DacWords;
    
    // num of bits
    static constexpr size_t kFlags = !kCompressNext ? (kCompressStrId ? 2 : 1) : 0;
    static constexpr size_t kFlagsExtend = kCompressNext ? (kCompressStrId ? 2 : 1) : 0;
    static constexpr size_t kBitsUpperNext = 8;
    static constexpr size_t kBitsUpperWords = kPlainWords ? 4 : 8;
    
    // mask
    static constexpr Base::storage_type kMaskIsFinal = 0b01;
    static constexpr Base::storage_type kMaskIsStrId = 0b10;
    static constexpr Base::storage_type kMaskFlags = (kFlags == 0) ? 0 : sim_ds::bit_util::width_mask<kFlags>;
    static constexpr Base::storage_type kMaskUpperNext = sim_ds::bit_util::width_mask<kBitsUpperNext>;
    static constexpr Base::storage_type kMaskUpperWords = sim_ds::bit_util::width_mask<kBitsUpperWords>;

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
    using RankSupportBitVector = sim_ds::SuccinctBitVector<false>;
    using DacVector = sim_ds::DacVector;
    using FitVector = sim_ds::FitVector;
    
private:
    // Enabled at NextCheck<true, C>
    RankSupportBitVector next_paths_;
    DacVector next_flow_;
    BitVector flags_bits_;
    // Enabled at NextCheck<N, true>
    RankSupportBitVector check_paths_;
    FitVector check_flow_;
    DacVector check_flow_dac_;
    // Used on mapping
    RankSupportBitVector words_paths_;
    FitVector words_flow_;
    RankSupportBitVector cum_words_paths_;
    FitVector cum_words_flow_;
    DacVector cum_words_dacs_;
    // Used on practical extraction
    RankSupportBitVector has_brother_bits_;
    std::vector<uint8_t> brother_;
    RankSupportBitVector is_state_bits_;
    std::vector<uint8_t> eldest_;
    
    /*******************************************/
    // Used for build.
    // These are not used after building.
    BitVector b_next_paths_;
    BitVector b_check_paths_;
    BitVector b_words_paths_;
    BitVector b_cum_words_paths_;
    BitVector b_has_brother_bits_;
    BitVector b_is_state_bits_;
    std::vector<id_type> b_next_flow_;
    std::vector<id_type> b_check_flow_;
    std::vector<id_type> b_words_flow_;
    std::vector<id_type> b_cum_words_flow_;
    /*******************************************/
    
    template <size_t Id>
    Base::storage_type front_byte_of_element_(size_t index) const {
        auto relative_pos = Base::element_table_[Id].pos;
        return Base::bytes_[Base::offset_(index) + relative_pos]; // Faster extraction
    }
    
    /**
     Flags that is_final and is_string_label are stored in upper bits of next.
     */
    uint8_t unmasked_flags_(size_t index) const {
        assert(!kCompressNext);
        return Base::bytes_[Base::offset_(index) + kElementPositionNext]; // Fastest extraction
    }
        
public:
    DoubleArrayImpl() = default;
    ~DoubleArrayImpl() = default;
    
    DoubleArrayImpl(const DoubleArrayImpl&) = delete;
    DoubleArrayImpl& operator=(const DoubleArrayImpl&) = delete;
    
    DoubleArrayImpl(DoubleArrayImpl&&) noexcept = default;
    DoubleArrayImpl& operator=(DoubleArrayImpl&&) noexcept = default;
    
    size_t next(size_t index) const {
        size_t ne = Base::get_(Base::offset_(index) + kElementPositionNext, Base::element_table_[kElementIdNext].size); // Faster extraction
        if constexpr (!kCompressNext) {
            return ne >> kFlags;
        } else {
            if (!next_paths_[index]) {
                return ne;
            } else {
                auto rank = next_paths_.rank(index);
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
            b_next_paths_[index] = has_flow;
            if (has_flow) {
                b_next_flow_.push_back(flow);
            }
        }
    }
    
    uint8_t check(size_t index) const {
        return Base::bytes_[Base::offset_(index) + kElementPositionCheck]; // Fastest extraction
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
            if constexpr (!kCompressNext)
                return static_cast<bool>(unmasked_flags_(index) & kMaskIsStrId);
            else
                return flags_bits_[index * kFlagsExtend + 1];
        } else {
            return check_paths_[index];
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
            b_check_paths_[index] = is_str;
        }
    }
    
    size_t string_id(size_t index) const {
        assert(kUseStrId);
        auto id = check(index);
        if (kCompressStrId and !check_paths_[index]) {
            return id;
        } else {
            auto rank = check_paths_.rank(index);
            if constexpr (kSelectId)
                return id | (check_flow_dac_[rank] << 8);
            else
                return id | (check_flow_[rank] << 8);
        }
    }
    
    void set_string_id(size_t index, size_t str_id) {
        assert(kUseStrId);
        set_check(index, str_id & 0xff);
        auto flow = str_id >> 8;
        if constexpr (kCompressStrId) {
            bool has_flow = flow > 0;
            b_check_paths_[index] = has_flow;
            if (has_flow)
                b_check_flow_.push_back(flow);
        } else {
            b_check_flow_.push_back(flow);
        }
    }
    
    size_t cum_words(size_t index) const {
        assert(kHashing);
        assert(kCumulativeWords);
        if constexpr (kCompressWords) {
            if constexpr (kDacWords) {
                return cum_words_dacs_[index];
            } else {
                if constexpr (kPlainWords) {
                    size_t cw = front_byte_of_element_<kElementIdCWords>(index) & kMaskUpperWords;
                    if (not cum_words_paths_[index]) {
                        return cw;
                    } else {
                        auto rank = cum_words_paths_.rank(index);
                        return cw | (cum_words_flow_[rank] << kBitsUpperWords);
                    }
                } else {
                    size_t cw = front_byte_of_element_<kElementIdCWords>(index);
                    if (not cum_words_paths_[index]) {
                        return cw;
                    } else {
                        auto rank = cum_words_paths_.rank(index);
                        return cw | (cum_words_flow_[rank] << kBitsUpperWords);
                    }
                }
            }
        } else {
            return Base::nested_element<kElementIdCWords>(index);
        }
    }
    
    void set_cum_words(size_t index, size_t cw) {
        assert(kHashing);
        assert(kCumulativeWords);
        
        if constexpr (kCompressWords) {
            if constexpr (kDacWords) {
                b_cum_words_flow_.push_back(cw);
            } else {
                if constexpr (kPlainWords) {
                    // Shared same byte with words to save each upper 4bits.
                    uint8_t base = Base::nested_element<kElementIdCWords>(index);
                    base &= kMaskUpperWords << kBitsUpperWords;
                    base |= cw & kMaskUpperWords;
                    Base::set_nested_element<kElementIdCWords>(index, base);
                    auto flow = cw >> kBitsUpperWords;
                    bool has_flow = flow > 0;
                    b_cum_words_paths_[index] = has_flow;
                    if (has_flow)
                        b_cum_words_flow_.push_back(flow);
                } else {
                    Base::set_nested_element<kElementIdCWords>(index, cw & kMaskUpperWords);
                    auto flow = cw >> kBitsUpperWords;
                    bool hasFlow = flow > 0;
                    b_cum_words_paths_[index] = hasFlow;
                    if (hasFlow)
                        b_cum_words_flow_.push_back(flow);
                }
            }
        } else {
            Base::set_nested_element<kElementIdCWords>(index, cw);
        }
    }
    
    size_t words(size_t index) const {
        assert(kHashing);
        assert(kPlainWords);
        if constexpr (kCompressWords) {
            size_t words = front_byte_of_element_<kElementIdCWords>(index) >> kBitsUpperWords;
            if (!words_paths_[index])
                return words;
            else {
                auto rank = words_paths_.rank(index);
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
            b_words_paths_[index] = has_flow;
            if (has_flow)
                b_words_flow_.push_back(flow);
        } else {
            Base::set_nested_element<kElementIdWords>(index, words);
        }
    }
    
    bool has_brother(size_t index) const {
        assert(kLinkChildren);
        return has_brother_bits_[index];
    }
    
    void set_has_brother(size_t index, bool has) {
        b_has_brother_bits_[index] = has;
    }
    
    uint8_t brother(size_t index) const {
        assert(kLinkChildren);
        assert(has_brother_bits_[index]);
        return brother_[has_brother_bits_.rank(index)];
    }
    
    void set_brother(size_t index, uint8_t bro) {
        brother_.push_back(bro);
    }
    
    bool is_state(size_t index) const {
        assert(kLinkChildren);
        return is_state_bits_[index];
    }
    
    void set_is_state(size_t index, bool is_state) {
        b_is_state_bits_[index] = is_state;
    }
    
    uint8_t eldest(size_t index) const {
        assert(kLinkChildren);
        assert(is_state_bits_[index]);
        return eldest_[is_state_bits_.rank(index)];
    }
    
    void set_eldest(size_t index, uint8_t eldest) {
        eldest_.push_back(eldest);
    }
    
    size_t num_elements() const {
        return Base::size();
    }
    
    // MARK: - Protocol settings for build
    
    // First. Set size of elements
    void resize(size_t size, size_t words = 0) {
        auto next_size = sim_ds::calc::SizeFitsInBytes(size << kFlags);
        std::vector<size_t> element_sizes = {1, kCompressNext ? 1 : next_size};
        if constexpr (kHashing) {
            if constexpr (kCompressWords and !kDacWords) {
                element_sizes.push_back(1);
            } else {
                auto words_size = sim_ds::calc::SizeFitsInBytes(words);
                element_sizes.push_back(words_size);
                if constexpr (kPlainWords and kCumulativeWords)
                    element_sizes.push_back(words_size);
            }
        }
        Base::set_element_sizes(element_sizes);
        Base::resize(size);
        if constexpr (kCompressNext) {
            b_next_paths_.resize(size);
            flags_bits_.resize(size * kFlagsExtend);
        }
        if constexpr (kUseStrId)
            b_check_paths_.resize(size);
        if constexpr (kHashing and kCompressWords) {
            b_cum_words_paths_.resize(size);
            if constexpr (kPlainWords)
                b_words_paths_.resize(size);
        }
        if constexpr (kLinkChildren) {
            b_has_brother_bits_.resize(size);
            b_is_state_bits_.resize(size);
        }
    }
    
    // Finaly. Serialize flows.
    void Freeze() {
        if constexpr (kCompressNext) {
            next_paths_ = RankSupportBitVector(b_next_paths_);
            next_flow_ = DacVector(b_next_flow_);
        }
        if constexpr (kUseStrId) {
            check_paths_ = RankSupportBitVector(b_check_paths_);
            if constexpr (kSelectId)
                check_flow_dac_ = DacVector(b_check_flow_);
            else
                check_flow_ = FitVector(b_check_flow_);
        }
        if constexpr (kHashing) {
            if constexpr (kPlainWords) {
                words_paths_ = RankSupportBitVector(b_words_paths_);
                words_flow_ = FitVector(b_words_flow_);
            }
            if constexpr (kDacWords) {
                cum_words_dacs_ = DacVector(b_cum_words_flow_);
            } else {
                cum_words_paths_ = RankSupportBitVector(b_cum_words_paths_);
                cum_words_flow_ = FitVector(b_cum_words_flow_);
            }
        }
        if constexpr (kLinkChildren) {
            has_brother_bits_ = RankSupportBitVector(b_has_brother_bits_);
            is_state_bits_ = RankSupportBitVector(b_is_state_bits_);
        }
        
        // Clear all build modules.
        std::apply([](auto*... args) {
            auto clear = [](auto* module) {
                module->resize(0);
                module->shrink_to_fit();
            };
            (clear(args), ...);
        }, std::make_tuple(&b_next_paths_,
                           &b_check_paths_,
                           &b_words_paths_,
                           &b_cum_words_paths_,
                           &b_has_brother_bits_,
                           &b_is_state_bits_,
                           &b_next_flow_,
                           &b_check_flow_,
                           &b_words_flow_,
                           &b_cum_words_flow_));
    }

    // MARK: - IO
    
    size_t size_in_bytes() const override {
        auto size = Base::size_in_bytes();
        if constexpr (kCompressNext) {
            size += next_paths_.size_in_bytes();
            size += next_flow_.size_in_bytes();
            size += flags_bits_.size_in_bytes();
        }
        if constexpr (kUseStrId) {
            size += check_paths_.size_in_bytes();
            if constexpr (kSelectId)
                size += check_flow_dac_.size_in_bytes();
            else
                size += check_flow_.size_in_bytes();
        }
        if constexpr (kHashing and kCompressWords) {
            if constexpr (kPlainWords) {
                size += words_paths_.size_in_bytes();
                size += words_flow_.size_in_bytes();
            }
            if constexpr (kCumulativeWords) {
                if constexpr (kDacWords) {
                    size += cum_words_dacs_.size_in_bytes();
                } else {
                    size += cum_words_paths_.size_in_bytes();
                    size += cum_words_flow_.size_in_bytes();
                }
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
            next_paths_.Read(is);
            next_flow_.Read(is);
            flags_bits_.Read(is);
        }
        if constexpr (kUseStrId) {
            check_paths_.Read(is);
            if constexpr (kSelectId)
                check_flow_dac_.Read(is);
            else
                check_flow_ = FitVector(is);
        }
        if constexpr (kHashing and kCompressWords) {
            if constexpr (kPlainWords) {
                words_paths_.Read(is);
                words_flow_ = FitVector(is);
            }
            if constexpr (kCumulativeWords) {
                if constexpr (kDacWords)
                    cum_words_dacs_.Read(is);
                else {
                    cum_words_paths_.Read(is);
                    cum_words_flow_ = FitVector(is);
                }
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
            next_paths_.Write(os);
            next_flow_.Write(os);
            flags_bits_.Write(os);
        }
        if constexpr (kUseStrId) {
            check_paths_.Write(os);
            if constexpr (kSelectId)
                check_flow_dac_.Write(os);
            else
                check_flow_.Write(os);
        }
        if constexpr (kHashing and kCompressWords) {
            if constexpr (kPlainWords) {
                words_paths_.Write(os);
                words_flow_.Write(os);
            }
            if constexpr (kCumulativeWords) {
                if constexpr (kDacWords) {
                    cum_words_dacs_.Write(os);
                } else {
                    cum_words_paths_.Write(os);
                    cum_words_flow_.Write(os);
                }
            }
        }
        if constexpr (kLinkChildren) {
            has_brother_bits_.Write(os);
            write_vec(brother_, os);
            is_state_bits_.Write(os);
            write_vec(eldest_, os);
        }
    }
    
    void ShowStats(std::ostream& os) const override {
        using std::endl;
        auto codes_name = [](bool use) {
            return use ? "Comp" : "Plain";
        };
        os << "--- Stat of " << "DoubleArrayImpl N:" << codes_name(kCompressNext) << "|C:" << codes_name(kUseStrId) << " ---" << endl;
        os << "size:\t" << Self::size_in_bytes() << endl;
        os << "\tbytes:\t" << Base::size_in_bytes() << endl;
        os << "\tnext:\t" << num_elements() * Base::element_size(kElementIdNext) + next_paths_.size_in_bytes() +  next_flow_.size_in_bytes() << endl;
        os << "\tcheck:\t" << num_elements() + check_paths_.size_in_bytes() + (!kSelectId ? check_flow_.size_in_bytes() : check_flow_dac_.size_in_bytes()) << endl;
        if constexpr (kHashing) {
            size_t cWordsSize;
            if constexpr (kPlainWords) {
                size_t wordsSize;
                if constexpr (kCumulativeWords) {
                    wordsSize = num_elements() / 2 + words_paths_.size_in_bytes() + words_flow_.size_in_bytes();
                    cWordsSize = (kDacWords ? cum_words_dacs_.size_in_bytes() : num_elements() / 2 + cum_words_paths_.size_in_bytes() + cum_words_flow_.size_in_bytes());
                } else {
                    wordsSize = num_elements() * Base::element_size(kElementIdWords);
                    cWordsSize = num_elements() * Base::element_size(kElementIdCWords);
                }
                os << "\twords:\t" << wordsSize << endl;
            } else {
                cWordsSize = (kCumulativeWords ? (kDacWords ? cum_words_dacs_.size_in_bytes() :
												  num_elements() + cum_words_paths_.size_in_bytes() + cum_words_flow_.size_in_bytes()) :
							  (num_elements() * Base::element_size(kElementIdCWords)));
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
    
    void ShowSizeMap(std::ostream& os) const {
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
    
};


//template<bool CompNext, bool CompCheck, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords, bool LinkChildren, bool SelectId, bool DacWords>
//void DoubleArrayImpl<CompNext, CompCheck, CompId, Hashing, CompWords, CumuWords, PlainWords, LinkChildren, SelectId, DacWords>::
//ShowStats(std::ostream& os) const {
//    using std::endl;
//    auto codes_name = [](bool use) {
//        return use ? "Comp" : "Plain";
//    };
//    os << "--- Stat of " << "DoubleArrayImpl N:" << codes_name(kCompressNext) << "|C:" << codes_name(kUseStrId) << " ---" << endl;
//    os << "size:\t" << Self::size_in_bytes() << endl;
//    os << "\tbytes:\t" << Base::size_in_bytes() << endl;
//    os << "\tnext:\t" << num_elements() * Base::element_size(kElementIdNext) + next_paths_.size_in_bytes() +  next_flow_.size_in_bytes() << endl;
//    os << "\tcheck:\t" << num_elements() + check_paths_.size_in_bytes() + (!kSelectId ? check_flow_.size_in_bytes() : check_flow_dac_.size_in_bytes()) << endl;
//    if constexpr (kHashing) {
//        size_t cWordsSize;
//        if constexpr (kPlainWords) {
//            size_t wordsSize;
//            if constexpr (kCumulativeWords) {
//                wordsSize = num_elements() / 2 + words_paths_.size_in_bytes() + words_flow_.size_in_bytes();
//                cWordsSize = (kDacWords ? cum_words_dacs_.size_in_bytes() : num_elements() / 2 + cum_words_paths_.size_in_bytes() + cum_words_flow_.size_in_bytes());
//            } else {
//                wordsSize = num_elements() * Base::element_size(kElementIdWords);
//                cWordsSize = num_elements() * Base::element_size(kElementIdCWords);
//            }
//            os << "\twords:\t" << wordsSize << endl;
//        } else {
//            cWordsSize = (kCumulativeWords ? (kDacWords ? cum_words_dacs_.size_in_bytes() :
//                                             num_elements() + cum_words_paths_.size_in_bytes() + cum_words_flow_.size_in_bytes()) :
//                          (num_elements() * Base::element_size(kElementIdCWords)));
//        }
//        os << "\tcum_words:\t" << cWordsSize << endl;
//
//        os << "---  ---" << endl;
//    }
//    if constexpr (kLinkChildren) {
//        os << "\tbrother:\t" << has_brother_bits_.size_in_bytes() + size_vec(brother_) << endl;
//        os << "\teldest:\t" << is_state_bits_.size_in_bytes() + size_vec(eldest_) << endl;
//        os << "---  ---" << endl;
//    }
//    //        showSizeMap(os);
//}

    
//template<bool CompNext, bool CompCheck, bool CompId, bool Hashing, bool CompWords, bool CumuWords, bool PlainWords, bool LinkChildren, bool SelectId, bool DacWords>
//void DoubleArrayImpl<CompNext, CompCheck, CompId, Hashing, CompWords, CumuWords, PlainWords, LinkChildren, SelectId, DacWords>::
//ShowSizeMap(std::ostream& os) const {
//    auto numElem = num_elements();
//    std::vector<size_t> nexts(numElem);
//    for (auto i = 0; i < numElem; i++)
//        nexts[i] = next(i) >> (!kCompressNext ? 1 : 0);
//
//    auto showList = [&](const std::vector<size_t>& list) {
//        using std::endl;
//        os << "-- " << "Next Map" << " --" << endl;
//        for (auto c : list)
//            os << c << "\t" << endl;
//        os << "/ " << numElem << endl;
//    };
//
//}
    
}

#endif /* Double_Array_Impr_hpp */

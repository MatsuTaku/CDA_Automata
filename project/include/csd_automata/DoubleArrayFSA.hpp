//
//  DoubleArrayFSA.hpp
//  DoubleArrayFSA
//
//  Created by 松本拓真 on 2018/01/13.
//

#ifndef DoubleArrayFSA_hpp
#define DoubleArrayFSA_hpp

#include "IOInterface.hpp"
#include "DAFoundation.hpp"
#include "sim_ds/BitVector.hpp"

#include "DoubleArrayFSABuilder.hpp"

namespace csd_automata {

template<bool CompressNext>
class DoubleArrayFSA : IOInterface {
public:
    static std::string name() {
        std::string name = (!kCompressNext ? "Original" : "Dac");
        return name + "DoubleArrayFSA";
    }
    
    static constexpr bool kCompressNext = CompressNext;
    using foundation_type = DAFoundation<CompressNext, false, false, false, false, false, false, false, false>;
    using BitVector = sim_ds::BitVector;
    
private:
    foundation_type fd_;
    BitVector is_final_bits_;
    size_t num_trans_ = 0;
    
public:
    DoubleArrayFSA(const DoubleArrayFSABuilder& builder) {
        Build(builder);
    }
    
    DoubleArrayFSA(std::istream &is) {
        LoadFrom(is);
    }
    
    void Build(DoubleArrayFSABuilder& builder);
    
    // MARK: - getter
    
    bool Accept(std::string_view str) const {
        size_t trans = 0;
        for (uint8_t c : str) {
            trans = target(trans) ^ c;
            if (check(trans) != c)
                return false;
        }
        return is_final(trans);
    }
    
    size_t Lookup(std::string_view str) const {
        return -1;
    }
    
    std::string Access(size_t key) const {
        return "";
    }
    
    auto target(size_t index) const {
        return next(index) ^ index;
    }
    
    auto next(size_t index) const {
        if constexpr (CompressNext)
            return fd_.next(index);
        else
            return fd_.next(index) >> 1;
    }
    
    auto check(size_t index) const {
        return fd_.check(index);
    }
    
    auto is_final(size_t index) const {
        if constexpr (CompressNext)
            return is_final_bits_[index];
        else
            return static_cast<bool>(fd_.next(index) & 1);
    }
    
    size_t words(size_t index) const {
        return -1;
    }
    
    size_t cum_words(size_t index) const {
        return -1;
    }
    
    // MARK: - Protocol setting
    
    void set_num_elements(size_t num) {
        fd_.resize(num);
        if (kCompressNext) {
            is_final_bits_.resize(num);
        }
    }
    
    void set_num_trans(size_t num) {
        num_trans_ = num;
    }
    
    // MARK: - ByteData method
    
    size_t size_in_bytes() const override {
        auto size = fd_.size_in_bytes();
        if constexpr (CompressNext)
            size += is_final_bits_.size_in_bytes();
        size += sizeof(num_trans_);
        return size;
    }
    
    void LoadFrom(std::istream& is) override {
        fd_.LoadFrom(is);
        if constexpr (CompressNext)
            is_final_bits_.Read(is);
        num_trans_ = read_val<size_t>(is);
    }
    
    void StoreTo(std::ostream& os) const override {
        fd_.StoreTo(os);
        if constexpr (CompressNext)
            is_final_bits_.Write(os);
        write_val(num_trans_, os);
    }
    
    void ShowStats(std::ostream& os) const override {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "#trans: " << num_trans_ << endl;
        os << "#elems: " << fd_.num_elements() << endl;
        os << "size:   " << size_in_bytes() << endl;
        os << "size is final:   " << is_final_bits_.size_in_bytes() << endl;
        fd_.ShowStats(os);
    }
    
    void PrintForDebug(std::ostream& os) const {
        
    }
    
    // MARK: - Copy guard
    
    DoubleArrayFSA() = default;
    ~DoubleArrayFSA() = default;
    
    DoubleArrayFSA (const DoubleArrayFSA&) = delete;
    DoubleArrayFSA& operator=(const DoubleArrayFSA&) = delete;
    
    DoubleArrayFSA(DoubleArrayFSA&&) noexcept = default;
    DoubleArrayFSA& operator=(DoubleArrayFSA&&) noexcept = default;
    
private:
    
    // MARK: - build
    
    void set_check(size_t index, uint8_t check) {
        fd_.set_check(index, check);
    }
    
    void set_next_and_is_final(size_t index, size_t next, bool isFinal) {
        if (CompressNext) {
            fd_.set_next(index, next);
            is_final_bits_[index] = isFinal;
        } else {
            fd_.set_next(index, (next << 1) | isFinal);
        }
    }
    
    void BuildBitArray() {
        if constexpr (!CompressNext) return;
        fd_.Build();
    }
    
};


template<bool CompressNext>
inline void DoubleArrayFSA<CompressNext>::Build(DoubleArrayFSABuilder& builder) {
    builder.Build();
    
    const auto numElem = builder.num_elements_();
    set_num_elements(numElem);
    
    auto numTrans = 0;
    for (auto i = 0; i < numElem; i++) {
        if (!builder.is_frozen_(i))
            continue;
        
        set_check(i, builder.get_check_(i));
        set_next_and_is_final(i, builder.get_next_(i), builder.is_final_(i));
        
        numTrans++;
    }
    set_num_trans(numTrans);
    BuildBitArray();
    
    builder.CheckEquivalence(*this);
}

}

#endif /* DoubleArrayFSA_hpp */

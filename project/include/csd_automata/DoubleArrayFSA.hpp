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
    
    class PlainFSA;
    
    template<bool N>
    class DoubleArrayFSA : IOInterface {
    public:
        static std::string name() {
            std::string name = (!useCodes ? "Original" : "Dac");
            return name + "DoubleArrayFSA";
        }
        
        static constexpr bool useCodes = N;
        using foundation_type = DAFoundation<N, false, false, false, false, false, false>;
        using bit_vector = sim_ds::BitVector;
        
    private:
        foundation_type fd_;
        bit_vector is_final_bits_;
        size_t num_trans_ = 0;
        
    public:
        DoubleArrayFSA(const PlainFSA& fsa) {
            build(fsa);
        }
        
        DoubleArrayFSA(std::istream &is) {
            Read(is);
        }
        
        void build(const PlainFSA &fsa);
        
        // MARK: - getter
        
        bool isMember(const std::string &str) const {
            size_t trans = 0;
            for (uint8_t c : str) {
                trans = target(trans) ^ c;
                if (check(trans) != c)
                    return false;
            }
            return isFinal(trans);
        }
        
        size_t lookup(const std::string &str) const {
            return -1;
        }
        
        std::string access(size_t key) const {
            return std::string("");
        }
        
        auto target(size_t index) const {
            return next(index) ^ index;
        }
        
        auto next(size_t index) const {
            if constexpr (N)
                return fd_.next(index);
            else
                return fd_.next(index) >> 1;
        }
        
        auto check(size_t index) const {
            return fd_.check(index);
        }
        
        auto isFinal(size_t index) const {
            if constexpr (N)
                return is_final_bits_[index];
            else
                return static_cast<bool>(fd_.next(index) & 1);
        }
        
        size_t store(size_t index) const {
            return -1;
        }
        
        size_t accStore(size_t index) const {
            return -1;
        }
        
        // MARK: - Protocol setting
        
        void setNumElement(size_t num) {
            fd_.resize(num);
            if (useCodes) {
                is_final_bits_.resize(num);
            }
        }
        
        void setNumTrans(size_t num) {
            num_trans_ = num;
        }
        
        // MARK: - ByteData method
        
        size_t size_in_Bytes() const override {
            auto size = fd_.size_in_Bytes();
            if constexpr (N)
                size += is_final_bits_.size_in_bytes();
            size += sizeof(num_trans_);
            return size;
        }
        
        void Write(std::ostream& os) const override {
            fd_.Write(os);
            if constexpr (N)
                is_final_bits_.Write(os);
            write_val(num_trans_, os);
        }
        
        void Read(std::istream& is) override {
            fd_.Read(is);
            if constexpr (N)
                is_final_bits_.Read(is);
            num_trans_ = read_val<size_t>(is);
        }
        
        void ShowStatus(std::ostream& os) const override {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << num_trans_ << endl;
            os << "#elems: " << fd_.num_elements() << endl;
            os << "size:   " << size_in_Bytes() << endl;
            os << "size is final:   " << is_final_bits_.size_in_bytes() << endl;
            fd_.ShowStatus(os);
        }
        
        void PrintForDebug(std::ostream &os) const {
            
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
        
        void setCheck(size_t index, uint8_t check) {
            fd_.set_check(index, check);
        }
        
        void setNextAndIsFinal(size_t index, size_t next, bool isFinal) {
            if (N) {
                fd_.set_next(index, next);
                is_final_bits_[index] = isFinal;
            } else {
                fd_.set_next(index, (next << 1) | isFinal);
            }
        }
        
        void buildBitArray() {
            if constexpr (!N) return;
            fd_.Build();
        }
        
    };
    
    
    template<bool N>
    inline void DoubleArrayFSA<N>::build(const PlainFSA &fsa) {
        DoubleArrayFSABuilder builder(fsa);
        builder.build();
        
        const auto numElem = builder.numElems_();
        setNumElement(numElem);
        
        auto numTrans = 0;
        for (auto i = 0; i < numElem; i++) {
            if (!builder.isFrozen_(i))
                continue;
            
            setCheck(i, builder.getCheck_(i));
            setNextAndIsFinal(i, builder.getNext_(i), builder.isFinal_(i));
            
            numTrans++;
        }
        setNumTrans(numTrans);
        buildBitArray();
        
        builder.showCompareWith(*this);
    }
    
}

#endif /* DoubleArrayFSA_hpp */

//
//  DoubleArrayFSA.hpp
//  DoubleArrayFSA
//
//  Created by 松本拓真 on 2018/01/13.
//

#ifndef DoubleArrayFSA_hpp
#define DoubleArrayFSA_hpp

#include "ByteData.hpp"

#include "DAFoundation.hpp"
#include "sim_ds/BitVector.hpp"

#include "ArrayFSABuilder.hpp"

namespace array_fsa {
    
    class PlainFSA;
    
    template<bool N>
    class DoubleArrayFSA : ByteData {
    public:
        DoubleArrayFSA() = default;
        
        DoubleArrayFSA(const PlainFSA &fsa) {
            build(fsa);
        }
        DoubleArrayFSA(std::istream &is) {
            read(is);
        }
        
        ~DoubleArrayFSA() = default;
        
        // MARK: - Copy guard
        
        DoubleArrayFSA (const DoubleArrayFSA&) = delete;
        DoubleArrayFSA& operator=(const DoubleArrayFSA&) = delete;
        
        DoubleArrayFSA(DoubleArrayFSA&&) noexcept = default;
        DoubleArrayFSA& operator=(DoubleArrayFSA&&) noexcept = default;
        
    public:
        static constexpr bool useCodes = N;
        using nc_type = DAFoundation<N, false, false, false, false, false>;
        
    public:
        static std::string name() {
            std::string name = (!useCodes ? "Original" : "Dac");
            return name + "DoubleArrayFSA";
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
                return nc_.next(index);
            else
                return nc_.next(index) >> 1;
        }
        
        auto check(size_t index) const {
            return nc_.check(index);
        }
        
        auto isFinal(size_t index) const {
            if constexpr (N)
                return is_final_bits_[index];
            else
                return (nc_.next(index) & 1) != 0;
        }
        
        size_t store(size_t index) const {
            return -1;
        }
        
        size_t accStore(size_t index) const {
            return -1;
        }
        
        // MARK: - Protocol setting
        
        void setNumElement(size_t num) {
            nc_.resize(num);
            if (useCodes) {
                is_final_bits_.resize(num);
            }
        }
        
        void setNumTrans(size_t num) {
            num_trans_ = num;
        }
        
        // MARK: - ByteData method
        
        size_t sizeInBytes() const override {
            auto size = nc_.sizeInBytes();
            if constexpr (N)
                size += is_final_bits_.sizeInBytes();
            size += sizeof(num_trans_);
            return size;
        }
        
        void write(std::ostream& os) const override {
            nc_.write(os);
            if constexpr (N)
                is_final_bits_.write(os);
            write_val(num_trans_, os);
        }
        
        void read(std::istream& is) override {
            nc_.read(is);
            if constexpr (N)
                is_final_bits_.read(is);
            num_trans_ = read_val<size_t>(is);
        }
        
        void showStatus(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << num_trans_ << endl;
            os << "#elems: " << nc_.numElements() << endl;
            os << "size:   " << sizeInBytes() << endl;
            os << "size is final:   " << is_final_bits_.sizeInBytes() << endl;
            nc_.showStatus(os);
        }
        
        void printForDebug(std::ostream &os) const {
            
        }
        
    private:
        nc_type nc_;
        sim_ds::BitVector is_final_bits_;
        size_t num_trans_ = 0;
        
        // MARK: - build
        
        void setCheck(size_t index, uint8_t check) {
            nc_.setCheck(index, check);
        }
        
        void setNextAndIsFinal(size_t index, size_t next, bool isFinal) {
            if (N) {
                nc_.setNext(index, next);
                is_final_bits_.set(index, isFinal);
            } else {
                nc_.setNext(index, (next << 1) | isFinal);
            }
        }
        
        void buildBitArray() {
            if constexpr (!N) return;
            nc_.build();
        }
        
    };
    
    
    template<bool N>
    inline void DoubleArrayFSA<N>::build(const PlainFSA &fsa) {
        ArrayFSABuilder builder(fsa);
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

//
//  FSA.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/13.
//

#ifndef FSA_hpp
#define FSA_hpp

#include "ByteData.hpp"

#include "NextCheck.hpp"
#include "sim_ds/BitVector.hpp"

#include "ArrayFSABuilder.hpp"

namespace array_fsa {
    
    class PlainFSA;
    
    template<bool N>
    class FSA : ByteData {
    public:
        static constexpr bool useCodes = N;
        using nc_type = NextCheck<N, false>;
        
        // MARK: - Copy guard
        
        FSA() = default;
        ~FSA() = default;
        
        FSA (const FSA&) = delete;
        FSA& operator=(const FSA&) = delete;
        
        FSA(FSA&&) noexcept = default;
        FSA& operator=(FSA&&) noexcept = default;
        
    public:
        static std::string name() {
            std::string name = (!useCodes ? "Original" : "Dac");
            return name + "FSA";
        }
        
        static FSA<N> build(const PlainFSA &fsa) {
            return ArrayFSABuilder::build<FSA<N>>(fsa);
        }
        
        // MARK: - getter
        
        bool isMember(const std::string& str) const {
            size_t trans = 0;
            for (uint8_t c : str) {
                trans = target(trans) ^ c;
                if (check(trans) != c)
                    return false;
            }
            return isFinal(trans);
        }
        
        auto target(size_t index) const {
            return next(index) ^ index;
        }
        
        auto next(size_t index) const {
            if (useCodes)
                return nc_.next(index);
            else
                return nc_.next(index) >> 1;
        }
        
        auto check(size_t index) const {
            return nc_.check(index);
        }
        
        auto isFinal(size_t index) const {
            if (useCodes)
                return is_final_bits_[index];
            else
                return (nc_.next(index) & 1) != 0;
        }
        
        // MARK: - build
        
        void setCheck(size_t index, uint8_t check) {
            nc_.setCheck(index, check);
        }
        
        void setNextAndIsFinal(size_t index, size_t next, bool isFinal) {
            if (useCodes) {
                nc_.setNext(index, next);
                is_final_bits_.set(index, isFinal);
            } else {
                nc_.setNext(index, (next << 1) | isFinal);
            }
        }
        
        void buildBitArray() {
            if (!useCodes) return;
            nc_.buildBitArray();
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
            if (useCodes)
                size += is_final_bits_.sizeInBytes();
            size += sizeof(num_trans_);
            return size;
        }
        
        void write(std::ostream& os) const override {
            nc_.write(os);
            if (useCodes)
                is_final_bits_.write(os);
            write_val(num_trans_, os);
        }
        
        void read(std::istream& is) override {
            nc_.read(is);
            if (useCodes)
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
        
    private:
        nc_type nc_;
        sim_ds::BitVector is_final_bits_;
        size_t num_trans_ = 0;
        
        friend class ArrayFSABuilder;
        
    };
    
    
    using OriginalFSA = FSA<false>;
    using DacFSA = FSA<true>;
    
    
}

#endif /* FSA_hpp */

//
//  StringTransFSA.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/14.
//

#ifndef StringTransFSA_hpp
#define StringTransFSA_hpp

#include "ByteData.hpp"

#include "NextCheck.hpp"
#include "Rank.hpp"
#include "StringArray.hpp"

namespace array_fsa {
    
    class PlainFSA;
    
    template <bool DAC>
    class StringTransFSA : ByteData {
    public:
        static std::string name() {
            std::string name = (!DAC ? "Original" : "Dac");
            return name + "STFSA";
        }
        
        static StringTransFSA build(const PlainFSA& fsa);
        
        StringTransFSA() {
            nc_.setUseDacNext(DAC);
            nc_.setUseDacCheck(true);
        }
        ~StringTransFSA() = default;
        
        bool isMember(const std::string& str) const { // TODO: -
            size_t trans = 0;
            for (size_t pos = 0; pos < str.size();) {
                uint8_t c = str[pos];
                trans = target(trans) ^ c;
                if (!isStringTrans(trans)) {
                    if (check(trans) != c) return false;
                    pos++;
                } else {
                    if (!strings_.isMatch(&pos, str, stringIndex(trans)))
                        return false;
                }
            }
            return isFinal(trans);
        }
        
        auto target(size_t index) const {
            return next(index) ^ index;
        }
        
        size_t next(size_t index) const {
            if (DAC)
                return nc_.next(index);
            else
                return nc_.next(index) >> 1;
        }
        
        uint8_t check(size_t index) const {
            return nc_.check(index);
        }
        
        size_t stringIndex(size_t index) const {
            auto i = nc_.check(index);
            return i;
        }
        
        bool isFinal(size_t index) const {
            if (DAC)
                return is_final_bits_.get(index);
            else
                return (nc_.next(index) & 1) != 0;
        }
        
        bool isStringTrans(size_t index) const {
//            return is_string_bits_.get(index);
            return nc_.getBitInFlow(index);
        }
        
        // MARK: - build
        
        void setCheck(size_t index, uint8_t check) {
            nc_.setCheck(index, check);
        }
        
        void setNextAndIsFinal(size_t index, size_t next, bool isFinal) {
            if (DAC) {
                nc_.setNext(index, next);
                is_final_bits_.set(index, isFinal);
            } else {
                size_t value = next << 1 | isFinal;
                nc_.setNext(index, value);
                if (nc_.next(index) != value) {
                    abort();
                }
            }
        }
        
        void setIsStringTrans(size_t index, bool isString) {
//            is_string_bits_.set(index, isString);
            nc_.setBitInFlow(index, isString);
        }
        
        void setStringIndex(size_t index, size_t strIndex) {
            nc_.setStringIndex(index, strIndex);
        }
        
        void setStringArray(std::vector<uint8_t> &strs) {
            strings_.setStringArray(strs);
        }
        
        void buildBitArray() {
            nc_.buildBitArray();
        }
        
        // MARK: - Protocol setting
        
        void setNumElement(size_t num) {
            if (DAC)
                nc_.setNumElement(num, false);
            else
                nc_.setNumElement(num, true);
        }
        
        void setNumStrings(size_t num) {
            nc_.setNumStrings(num);
        }
        
        void setNumTrans(size_t num) {
            num_trans_ = num;
        }
        
        // MARK: - ByteData method
        
        size_t sizeInBytes() const override {
            auto size = sizeof(num_trans_);
            size += nc_.sizeInBytes();
            if (DAC)
                size += is_final_bits_.sizeInBytes();
            size += is_string_bits_.sizeInBytes();
            size += strings_.sizeInBytes();
            return size;
        }
        
        void write(std::ostream& os) const override {
            write_val(num_trans_, os);
            nc_.write(os);
            if (DAC)
                is_final_bits_.write(os);
            is_string_bits_.write(os);
            strings_.write(os);
        }
        
        void read(std::istream& is) override {
            num_trans_ = read_val<size_t>(is);
            nc_.read(is);
            if (DAC)
                is_final_bits_.read(is);
            is_string_bits_.read(is);
            strings_.read(is);
        }
        
        void show_stat(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << num_trans_ << endl;
            os << "#elems: " << nc_.numElements() << endl;
            os << "size:   " << sizeInBytes() << endl;
            os << "size is final:   " << is_final_bits_.sizeInBytes() << endl;
            os << "size is string:  " << is_string_bits_.sizeInBytes() << endl;
            os << "size strings:    " << strings_.sizeInBytes() << endl;
            nc_.showStatus(os);
        }
        
        // MARK: - Copy guard
        
        StringTransFSA (const StringTransFSA&) = delete;
        StringTransFSA& operator =(const StringTransFSA&) = delete;
        
        StringTransFSA(StringTransFSA&& rhs) noexcept = default;
        StringTransFSA& operator =(StringTransFSA&& rhs) noexcept = default;
        
    private:
        size_t num_trans_ = 0;
        NextCheck<DACs> nc_;
        Rank is_final_bits_;
        Rank is_string_bits_;
        StringArray strings_;
        
    };
    
    using STFSA = StringTransFSA<false>;
    using DacSTFSA = StringTransFSA<true>;
    
}

#endif /* StringTransFSA_hpp */

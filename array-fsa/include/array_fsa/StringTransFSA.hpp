
//  StringTransFSA.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/14.
//

#ifndef StringTransFSA_hpp
#define StringTransFSA_hpp

#include "ByteData.hpp"

#include "NextCheck.hpp"
#include "StringArray.hpp"
#include "sim_ds/BitVector.hpp"

#include "ArrayFSATailBuilder.hpp"

namespace array_fsa {
    
    class PlainFSA;
    
    class StringTransFSA : ByteData {
    public:
        StringTransFSA() = default;
        
        explicit StringTransFSA(const PlainFSA &fsa) {
            build(fsa);
        }
        
        explicit StringTransFSA(std::istream &is) {
            read(is);
        }
        
        ~StringTransFSA() = default;
        
        // MARK: - Copy guard
        
        StringTransFSA (const StringTransFSA&) = delete;
        StringTransFSA& operator=(const StringTransFSA&) = delete;
        
        StringTransFSA(StringTransFSA&& rhs) noexcept = default;
        StringTransFSA& operator=(StringTransFSA&& rhs) noexcept = default;
        
    public:
        static constexpr bool useBinaryLabel = false;
        
        using nc_type = NextCheck<false, true, true>;
        using sa_type = StringArray<useBinaryLabel>;
        
    public:
        static std::string name() {
            return "StringTransFSA";
        }
        
        void build(const PlainFSA &fsa) {
            const auto shouldMergeSuffix = false;
            ArrayFSATailBuilder builder(fsa);
            builder.ArrayFSATailBuilder::build(useBinaryLabel, shouldMergeSuffix);
            
            const auto numElems = builder.numElems_();
            resize(numElems, fsa.get_num_words());
            strings_ = sa_type(builder.getStringArrayBuilder());
            
            auto numTrans = 0;
            for (auto i = 0; i < numElems; i++) {
                setNextAndIsFinal(i, builder.getNext_(i), builder.isFinal_(i));
                setCheck(i, builder.getCheck_(i));
                auto isStrTrans = builder.hasLabel_(i);
                setIsStringTrans(i, isStrTrans);
                if (isStrTrans)
                    setStringIndex(i, builder.getLabelNumber_(i));
                else
                    setCheck(i, builder.getCheck_(i));
                
                setStore(i, builder.getStore_(i));
                setAccStore(i, builder.getAccStore_(i));
                
                if (builder.isFrozen_(i))
                    numTrans++;
            }
            buildBitArray();
            setNumTrans(numTrans);
            
            builder.showCompareWith(*this);
            
        }
        
        bool isMember(const std::string &str) const {
            size_t trans = 0;
            for (size_t pos = 0, size = str.size(); pos < size;) {
                uint8_t c = str[pos];
                trans = target(trans) ^ c;
                if (!isStringTrans(trans)) {
                    auto checkE = check(trans);
                    if (checkE != c)
                        return false;
                    pos++;
                } else {
                    auto sid = stringId(trans);
                    if (!strings_.isMatch(&pos, str, sid)) {
                        strings_.showLabels(sid - 32, sid + 32);
                        return false;
                    }
                }
            }
            return isFinal(trans);
        }
        
        size_t target(size_t index) const {
            return next(index) ^ index;
        }
        
        size_t next(size_t index) const {
            return nc_.next(index) >> 1;
        }
        
        uint8_t check(size_t index) const {
            return nc_.check(index);
        }
        
        size_t stringId(size_t index) const {
            return nc_.stringId(index);
        }
        
        bool isFinal(size_t index) const {
            return (nc_.next(index) & 1) != 0;
        }
        
        bool isStringTrans(size_t index) const {
            return is_string_bits_[index];
        }
        
        // MARK: - ByteData method
        
        size_t sizeInBytes() const override {
            auto size = sizeof(num_trans_);
            size += nc_.sizeInBytes();
            size += is_string_bits_.sizeInBytes();
            size += strings_.sizeInBytes();
            return size;
        }
        
        void write(std::ostream& os) const override {
            write_val(num_trans_, os);
            nc_.write(os);
            is_string_bits_.write(os);
            strings_.write(os);
        }
        
        void writeCheck(std::ostream &os) const {
            std::vector<size_t> checks(nc_.numElements());
            for (auto i = 0; i < nc_.numElements(); i++) {
                checks[i] = isStringTrans(i) ? nc_.stringId(i) : nc_.check(i);
            }
            sim_ds::Vector vec(checks);
            vec.write(os);
        }
        
        void read(std::istream& is) override {
            num_trans_ = read_val<size_t>(is);
            nc_.read(is);
            is_string_bits_.read(is);
            strings_.read(is);
        }
        
        void showStatus(std::ostream& os) const {
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
        
    private:
        size_t num_trans_ = 0;
        nc_type nc_;
        sim_ds::BitVector is_final_bits_;
        sim_ds::BitVector is_string_bits_;
        sa_type strings_;
        
        // MARK: - Protocol setting
        
        void resize(size_t num, size_t words) {
            nc_.resize(num, words);
        }
        
        void setNumTrans(size_t num) {
            num_trans_ = num;
        }
        
        // MARK: - build
        
        void setCheck(size_t index, uint8_t check) {
            nc_.setCheck(index, check);
        }
        
        void setNextAndIsFinal(size_t index, size_t next, bool isFinal) {
            size_t value = next << 1 | isFinal;
            nc_.setNext(index, value);
        }
        
        void setIsStringTrans(size_t index, bool isString) {
            is_string_bits_.set(index, isString);
        }
        
        void setStringIndex(size_t index, size_t strIndex) {
            nc_.setStringId(index, strIndex);
        }
        
        void setStringArray(sa_type &&sArr) {
            strings_ = std::move(sArr);
        }
        
        void setStore(size_t index, size_t store) {
            nc_.setStore(index, store);
        }
        
        void setAccStore(size_t index, size_t as) {
            nc_.setAccStore(index, as);
        }
        
        void buildBitArray() {
            nc_.buildBitArray();
        }
        
    };
    
}

#endif /* StringTransFSA_hpp */

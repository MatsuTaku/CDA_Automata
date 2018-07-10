//
//  ArrayFSATailBuilder.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#ifndef ArrayFSA_TailBuilder_hpp
#define ArrayFSA_TailBuilder_hpp

#include "ArrayFSABuilder.hpp"

#include "StringDictBuilder.hpp"
#include "StringArray.hpp"

namespace array_fsa {
    
    class ArrayFSATailBuilder : public ArrayFSABuilder {
    public:
        template <class T>
        static T build(const PlainFSA &origFsa);
        
        template <class T>
        void showCompareWith(T &fsa);
        
    protected:
        StringDict str_dict_;
        
        explicit ArrayFSATailBuilder(const PlainFSA &orig_fsa, bool binaryMode, bool mergeSuffix) : ArrayFSABuilder(orig_fsa) {
            str_dict_ = StringDictBuilder::build(orig_fsa_, binaryMode, mergeSuffix);
            build_(binaryMode);
        }
        
        virtual ~ArrayFSATailBuilder() = default;
        
        size_t getLabelNumber_(size_t index) const {
            size_t labelIndex = 0;
            std::memcpy(&labelIndex, &bytes_[offset_(index) + 1 + kAddrSize], 4);
            return labelIndex;
        }
        
        bool hasLabel_(size_t index) const {
            return (bytes_[offset_(index)] & 8) != 0;
        }
        
        bool isLabelEnd_(size_t index) const {
            return str_dict_.isEndLabel(index);
        }
        
        void setLabelIndex_(size_t index, size_t labelIndex) {
            std::memcpy(&bytes_[offset_(index) + 1 + kAddrSize], &labelIndex, 4);
        }
        
        void setHasLabel_(size_t index) {
            bytes_[offset_(index)] |= 8;
        }
        
        void build_(bool binaryMode);
        void arrange_(size_t state, size_t index) override;
        
    };
    
    
    template <class T>
    T ArrayFSATailBuilder::build(const PlainFSA &origFsa) {
        const auto isBinary = T::useBinaryLabel;
        const auto shouldMergeSuffix = false;
        ArrayFSATailBuilder builder(origFsa, isBinary, shouldMergeSuffix);
        
        // Release
        T newFsa;
        
        const auto numElems = builder.numElems_();
        newFsa.setNumElement(numElems);
        newFsa.strings_ = StringArray<isBinary>(&labelArray(builder.str_dict_));
        
        auto numTrans = 0;
        for (auto i = 0; i < numElems; i++) {
            newFsa.setNextAndIsFinal(i, builder.getNext_(i), builder.isFinal_(i));
            newFsa.setCheck(i, builder.getCheck_(i));
            auto isStrTrans = builder.hasLabel_(i);
            newFsa.setIsStringTrans(i, isStrTrans);
            if (isStrTrans)
                newFsa.setStringIndex(i, builder.getLabelNumber_(i));
            else
                newFsa.setCheck(i, builder.getCheck_(i));
            
            if (builder.isFrozen_(i))
                numTrans++;
        }
        newFsa.buildBitArray();
        newFsa.setNumTrans(numTrans);
        
        builder.showCompareWith(newFsa);
        
        return newFsa;
    }
    
    template <class T>
    void ArrayFSATailBuilder::showCompareWith(T &fsa) {
        auto tab = "\t";
        for (auto i = 0; i < numElems_(); i++) {
            auto bn = getNext_(i);
            auto bi = hasLabel_(i);
            auto bc = !bi ? getCheck_(i) : getLabelNumber_(i);
            auto fn = fsa.next(i);
            auto fi = fsa.isStringTrans(i);
            auto fc = !fi ? fsa.check(i) : fsa.stringId(i);
            if (bn == fn && bc == fc && bi == fi)
                continue;
            using std::cout, std::endl;
            cout << i << "] builder" << tab << "fsa" << endl;
            cout << "next: " << bn << tab << fn << endl;
            cout << "check: " << bc << tab << fc << endl;
            cout << "is-str: " << bi << tab << fi << endl;
            cout << "accept: " << isFinal_(i) << tab << fsa.isFinal(i) << endl;
//            std::cout << i << tab << isFinal_(i) << tab << bn << tab << bc << tab << bi << std::endl;
//            std::cout << i << tab << fsa.isFinal(i) << tab << fn << tab << fc << tab << fi << std::endl;
            if (bi || fi) {
                sim_ds::Log::showAsBinary(bc, 4);
                sim_ds::Log::showAsBinary(fc, 4);
            }
            std::cout << std::endl;
        }
        
    }
    
}

#endif /* ArrayFSA_TailBuilder_hpp */

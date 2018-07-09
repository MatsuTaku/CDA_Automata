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
        void showInBox(T &fsa);
        
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
        //            newFsa.setNumStrings(newFsa.strings_.size());
        
        auto numTrans = 0;
        for (auto i = 0; i < numElems; i++) {
            newFsa.setNextAndIsFinal(i, builder.getNext_(i), builder.isFinal_(i));
            newFsa.setCheck(i, builder.getCheck_(i));
            auto isStrTrans = builder.hasLabel_(i);
            newFsa.setIsStringTrans(i, isStrTrans);
            if (isStrTrans) {
                newFsa.setStringIndex(i, builder.getLabelNumber_(i));
            } else {
                newFsa.setCheck(i, builder.getCheck_(i));
            }
            
            if (builder.isFrozen_(i))
                numTrans++;
        }
        newFsa.buildBitArray();
        newFsa.setNumTrans(numTrans);
        
        //    showInBox(builder, newFsa);
        
        return newFsa;
    }
    
    template <class T>
    void ArrayFSATailBuilder::showInBox(T &fsa) {
        auto tab = "\t";
        auto start = 0;
        for (auto i = start; i < start + 0x100; i++) {
            std::cout << i << tab << isFinal_(i) << tab << getNext_(i) << tab << getCheck_(i) << tab << hasLabel_(i) << std::endl;
            std::cout << i << tab << fsa.isFinal(i) << tab << fsa.next(i) << tab << fsa.check(i) << tab << fsa.isStringTrans(i) << std::endl;
            if (hasLabel_(i)) {
                sim_ds::Log::showAsBinary(getLabelNumber_(i), 4);
                sim_ds::Log::showAsBinary(fsa.stringId(i), 4);
            }
            std::cout << std::endl;
        }
    }
    
}

#endif /* ArrayFSA_TailBuilder_hpp */

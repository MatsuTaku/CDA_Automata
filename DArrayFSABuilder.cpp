//
//  DArrayFSABuilder.cpp
//  bouble_array
//
//  Created by 松本拓真 on 2017/12/11.
//

#include "ArrayFSABuilder.hpp"

#include "DArrayFSA.hpp"

using namespace double_array;

DArrayFSA array_fsa::ArrayFSABuilder::buildD(const array_fsa::PlainFSA &orig_fsa, const DArrayFSAAccessoryTypes &types) {
    array_fsa::ArrayFSABuilder builder(orig_fsa);
    builder.build_();
    
    // Release
    DArrayFSA newFsa;
    newFsa.setTypes(types);
    newFsa.initMembers();
    newFsa.generateName();
    
    const auto numElems = builder.num_elems_();
    newFsa.calcNextSize(numElems << 1); // TODO: - OMG
    const auto elementSize = newFsa.getNextSize() + 1;
    newFsa.setElementSize(elementSize);
    newFsa.resizeBytes(numElems * elementSize);
    
    auto numTrans = 0;
    for (size_t i = 0; i < numElems; ++i) {
        newFsa.setNext(i, builder.get_next_(i));
        newFsa.setCheck(i, builder.get_check_(i));
        newFsa.setIsFinal(i, builder.is_final_(i));
        
        if (builder.is_frozen_(i)) {
            numTrans++;
        }
    }
    newFsa.setNumTrans(numTrans);
    
//    builder.showMapping(false);
    showInBox(builder, newFsa);
    
    return newFsa;
}

void array_fsa::ArrayFSABuilder::showInBox(array_fsa::ArrayFSABuilder &builder, double_array::DArrayFSA &fsa) {
    
    auto tab = "\t";
    for (auto i = 0; i < 0x100; i++) {
        std::cout << i << tab << builder.is_final_(i) << tab << builder.get_next_(i) << tab << builder.get_check_(i) << std::endl;
        std::cout << i << tab << fsa.getIsFinal(i) << tab << fsa.getNext(i) << tab << fsa.getCheck(i) << std::endl;;
        std::cout << std::endl;
    }
}

//
//  NArrayFSABuilder.cpp
//  bench
//
//  Created by 松本拓真 on 2018/01/05.
//

#include "ArrayFSABuilder.hpp"

#include "NArrayFSA.hpp"
#include "Calc.hpp"

using namespace array_fsa;

NArrayFSA ArrayFSABuilder::buildN(const PlainFSA &orig_fsa) {
    ArrayFSABuilder builder(orig_fsa);
    builder.build_();
    
    // Release
    NArrayFSA newFsa;
    
    const auto numElems = builder.num_elems_();
    auto nextSize = Calc::sizeFitInBytes(numElems << 1);
    newFsa.byte_array_.insertValueSize(0, nextSize);
    newFsa.byte_array_.insertValueSize(1, 1);
    newFsa.byte_array_.resize(numElems);
    
    auto numTrans = 0;
    for (size_t i = 0; i < numElems; ++i) {
//        newFsa.setNext(i, builder.get_next_(i));
        newFsa.setCheck(i, builder.get_check_(i));
//        newFsa.setIsFinal(i, builder.is_final_(i));
        newFsa.setNextAndFinal(i, builder.get_next_(i), builder.is_final_(i));
        
        if (builder.is_frozen_(i)) {
            numTrans++;
        }
    }
    newFsa.set_num_trans_(numTrans);
    
//    showInBox(builder, newFsa);
    
    return newFsa;
}

template <typename T>
void ArrayFSABuilder::showInBox(ArrayFSABuilder &builder, T &fsa) {
    auto tab = "\t";
    for (auto i = 0; i < 0x100; i++) {
        std::cout << i << tab << builder.is_final_(i) << tab << builder.get_next_(i) << tab << builder.get_check_(i) << std::endl;
        std::cout << i << tab << fsa.isFinal(i) << tab << fsa.getNext(i) << tab << fsa.getCheck(i) << std::endl;;
        std::cout << std::endl;
    }
}

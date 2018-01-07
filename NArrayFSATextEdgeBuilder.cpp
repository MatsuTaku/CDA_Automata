//
//  NArrayFSATextEdgeBuilder.cpp
//  array_fsa
//
//  Created by 松本拓真 on 2018/01/07.
//

#include "ArrayFSATailBuilder.hpp"

#include "PlainFSA.hpp"
#include "NArrayFSATextEdge.hpp"

using namespace array_fsa;

template <>
NArrayFSATextEdge ArrayFSATailBuilder::build<NArrayFSATextEdge>(const array_fsa::PlainFSA &origFsa) {
    ArrayFSATailBuilder builder(origFsa);
    builder.build_();
    
    // Release
    NArrayFSATextEdge newFsa;
    
    const auto numElems = builder.num_elems_();
    auto nextSize = Calc::sizeFitInBytes((numElems - 1) << 1);
    newFsa.setValuesSizes(nextSize, 1);
    newFsa.byte_array_.resize(numElems);
    newFsa.labels_ = builder.str_dict_.get_label_bytes();
    auto labelIndexSize = Calc::sizeFitInBytes(newFsa.labels_.size() - 1);
    newFsa.label_index_flows_.setUnitSize(labelIndexSize - 1);
    newFsa.label_index_flows_.setMaxValue(newFsa.labels_.size() - 1);
    newFsa.label_index_flows_.useLink(true);
    
    for (size_t i = 0; i < numElems; ++i) {
        if (!builder.has_label(i)) {
            newFsa.setCheck(i, builder.get_check_(i));
        } else {
            newFsa.setHasLabel(i, true);
            newFsa.setLabelIndex(i, builder.get_label_number(i));
        }
        newFsa.setNextAndFinal(i, builder.get_next_(i), builder.is_final_(i));
        
        if (builder.is_frozen_(i)) {
            newFsa.num_trans_++;
        }
    }
    newFsa.label_index_flows_.build();
    
//    showInBox(builder, newFsa);
    
    return newFsa;
}

template <typename T>
void ArrayFSATailBuilder::showInBox(array_fsa::ArrayFSATailBuilder &builder, T &fsa) {
    auto tab = "\t";
    for (auto i = 0; i < 0x100; i++) {
        std::cout << i << tab << builder.is_final_(i) << tab << builder.get_next_(i) << tab << builder.get_check_(i) << tab << builder.has_label(i) << std::endl;
        std::cout << i << tab << fsa.isFinal(i) << tab << fsa.getNext(i) << tab << fsa.getCheck(i) << tab << fsa.hasLabel(i) << std::endl;;
//        Rank::show_as_bytes(builder.get_label_number(i), 4);
//        Rank::show_as_bytes(fsa.getLabelIndex(i), 4);
        std::cout << std::endl;
    }
}

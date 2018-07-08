//
//  FSA.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/13.
//

#include "array_fsa/FSA.hpp"

#include "array_fsa/ArrayFSABuilder.hpp"

using namespace array_fsa;


template<>
OriginalFSA OriginalFSA::build(const PlainFSA& fsa) {
    return ArrayFSABuilder::build<OriginalFSA>(fsa);
}

template<>
DacFSA DacFSA::build(const PlainFSA& fsa) {
    return ArrayFSABuilder::build<DacFSA>(fsa);
}


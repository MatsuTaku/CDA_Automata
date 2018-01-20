//
//  FSA.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/13.
//

#include "FSA.hpp"

#include "ArrayFSABuilder.hpp"

using namespace array_fsa;

template <>
FSA<true> FSA<true>::build(const PlainFSA& fsa) {
    return ArrayFSABuilder::build<true>(fsa);
}

template <>
FSA<false> FSA<false>::build(const PlainFSA& fsa) {
    return ArrayFSABuilder::build<false>(fsa);
}

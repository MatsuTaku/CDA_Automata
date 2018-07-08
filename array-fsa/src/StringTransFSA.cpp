//
//  StringTransFSA.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/14.
//

#include "array_fsa/StringTransFSA.hpp"

#include "array_fsa/ArrayFSATailBuilder.hpp"

using namespace array_fsa;

template<>
STFSA STFSA::build(const PlainFSA &fsa) {
    return ArrayFSATailBuilder::build<STFSA>(fsa);
}


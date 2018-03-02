//
//  StringTransFSA.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/14.
//

#include "StringTransFSA.hpp"

#include "ArrayFSATailBuilder.hpp"

using namespace array_fsa;

template<>
STFSA STFSA::build(const PlainFSA &fsa) {
    return ArrayFSATailBuilder::build<STFSA>(fsa);
}
template<>
STCFSA STCFSA::build(const PlainFSA &fsa) {
    return ArrayFSATailBuilder::build<STCFSA>(fsa);
}
template<>
STCFSAB STCFSAB::build(const PlainFSA &fsa) {
    return ArrayFSATailBuilder::build<STCFSAB>(fsa);
}


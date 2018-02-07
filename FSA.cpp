//
//  FSA.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/13.
//

#include "FSA.hpp"

#include "ArrayFSABuilder.hpp"

using namespace array_fsa;

template<> class FSA<false>;
template<> class FSA<true, DACs>;
template<> class FSA<true, SACs>;

template <>
FSA<false> FSA<false>::build(const PlainFSA& fsa) {
    return ArrayFSABuilder::build<false, DACs>(fsa);
}

template <>
FSA<true, DACs> FSA<true, DACs>::build(const PlainFSA& fsa) {
    return ArrayFSABuilder::build<true, DACs>(fsa);
}

template<>
FSA<true, SACs> FSA<true, SACs>::build(const PlainFSA& fsa) {
    return ArrayFSABuilder::build<true, SACs>(fsa);
}


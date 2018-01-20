//
//  StringTransFSA.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/14.
//

#include "StringTransFSA.hpp"

#include "ArrayFSATailBuilder.hpp"

using namespace array_fsa;

template<> class StringTransFSA<false>;
template<> class StringTransFSA<true>;

//template <>
//StringTransFSA<true> FSA<true>::build(const PlainFSA& fsa) {
//    return ArrayFSABuilder::build<true>(fsa);
//}

template <>
StringTransFSA<false> StringTransFSA<false>::build(const PlainFSA& fsa) {
    return ArrayFSATailBuilder::build<false>(fsa);
}
template <>
StringTransFSA<true> StringTransFSA<true>::build(const PlainFSA& fsa) {
    return ArrayFSATailBuilder::build<true>(fsa);
}


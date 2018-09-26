//
//  array_fsa.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/07/05.
//

#ifndef array_fsa_hpp
#define array_fsa_hpp

#include "array_fsa/DoubleArrayFSA.hpp"
#include "array_fsa/DoubleArrayCFSA.hpp"
#include "array_fsa/MorfologikCFSA2.hpp"
#include "array_fsa/MorfologikFSADictionary.hpp"

namespace array_fsa {
    
    using OriginalFSA = DoubleArrayFSA<false>;
    using DacFSA = DoubleArrayFSA<true>;
    using MorfologikFSA = MorfologikFSADictionary<MorfologikFSA5DictionaryFoundation>;
    
}

#endif /* array_fsa_hpp */

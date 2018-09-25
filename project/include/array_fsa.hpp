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
#include "array_fsa/MorfologikFSA5Dictionary.hpp"

namespace array_fsa {
    
    using OriginalFSA = DoubleArrayFSA<false>;
    using DacFSA = DoubleArrayFSA<true>;
    
}

#endif /* array_fsa_hpp */
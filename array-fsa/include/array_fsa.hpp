//
//  array_fsa.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/07/05.
//

#ifndef array_fsa_hpp
#define array_fsa_hpp

#include "array_fsa/FSA.hpp"
#include "array_fsa/StringTransFSA.hpp"
#include "array_fsa/MorfologikCFSA2.hpp"
#include "array_fsa/MorfologikFSA5.hpp"

namespace array_fsa {
    
    using OriginalFSA = FSA<false>;
    using DacFSA = FSA<true>;
    
}

#endif /* array_fsa_hpp */

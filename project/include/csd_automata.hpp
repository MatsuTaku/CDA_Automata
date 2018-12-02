//
//  csd_automata.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/07/05.
//

#ifndef array_fsa_hpp
#define array_fsa_hpp

#include "csd_automata/DoubleArrayFSA.hpp"
#include "csd_automata/DoubleArrayCFSA.hpp"
#include "csd_automata/MorfologikFsaDictionary.hpp"

namespace csd_automata {

using DaFsa = DoubleArrayFSA<false>;
using DaFsaDac = DoubleArrayFSA<true>;

using SdDaFsa = DoubleArrayCFSA<true, true, false, true, true>;
using SdLoDaFsa = DoubleArrayCFSA<true, false, false, true, false>;
    
}

#endif /* array_fsa_hpp */

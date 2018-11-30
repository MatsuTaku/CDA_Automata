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
#include "csd_automata/MorfologikFSADictionary.hpp"

namespace csd_automata {

using OriginalFSA = DoubleArrayFSA<false>;
using DacFSA = DoubleArrayFSA<true>;

using DoubleArrayAutomataDictionary = DoubleArrayCFSA<true, true, false, true, true>;
using DoubleArrayAutomataLookupDictionary = DoubleArrayCFSA<true, false, false, true, false>;

using DAMTypes = std::tuple<
DoubleArrayAutomataLookupDictionary,
DoubleArrayAutomataDictionary
>;
    
}

#endif /* array_fsa_hpp */

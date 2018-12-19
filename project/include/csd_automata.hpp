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

// Recomended
using Dam = DoubleArrayCFSA<true, true, false, false, true, false, false>;

// For Expression
using SdDaFsa = DoubleArrayCFSA<true, true, true, false, true, true, false>;
using SdLoDaFsa = DoubleArrayCFSA<true, true, false, false, true, false, false>;
using SdLoDacDaFsa = DoubleArrayCFSA<true, true, false, false, true, false, true>;
using SdLoCidDaFsa = DoubleArrayCFSA<true, true, false, true, true, false, false>;
// Rejected
using SdLoSiDaFsa = DoubleArrayCFSA<false, true, false, true, true, false, false>;

}

#endif /* array_fsa_hpp */

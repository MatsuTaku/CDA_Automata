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
using Dam = DoubleArrayCFSA<true, true, false, false, true, false, false, false, false>;

// For Expression
using SdDaFsa = DoubleArrayCFSA<true, true, true, false, true, true, false, false, false>;
using SdLoDaFsa = DoubleArrayCFSA<true, true, false, false, true, false, false, false, false>;
using SdLoCnDaFsa = DoubleArrayCFSA<true, true, false, false, true, false, true, false, false>;
using SdLoCidDaFsa = DoubleArrayCFSA<true, true, false, true, true, false, false, false, false>;
using SdLoCsidDaFsa = DoubleArrayCFSA<true, true, false, true, true, false, false, true, false>;
using SdLoCnsidDaFsa = DoubleArrayCFSA<true, true, false, true, true, false, true, true, false>;
using SdLoDwDaFsa = DoubleArrayCFSA<true, true, false, false, true, false, false, false, true>;
// Rejected
using SdLoSiDaFsa = DoubleArrayCFSA<false, true, false, true, true, false, false, false, false>;

}

#endif /* array_fsa_hpp */

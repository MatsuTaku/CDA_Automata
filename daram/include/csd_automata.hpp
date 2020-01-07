//
//  csd_automata.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/07/05.
//

#ifndef array_fsa_hpp
#define array_fsa_hpp

#include "csd_automata/Dawg.hpp"
#include "csd_automata/Cdawg.hpp"
#include "csd_automata/MorfologikFsaDictionary.hpp"

namespace csd_automata {

using DaFsa = Dawg<false, false, false, false>;
using DaFsaDac = Dawg<true, false, false, false>;

// Recomended
using Daram = Cdawg<true, false, false, true, false, false, false, false>;

// For Experiments
// DAWG
using SdDaDawg = Dawg<false, true, false, false>;
using SdDaDawgCW = Dawg<false, true, false, true>;
// CDAWG
using SdDaFsa = Cdawg<true, true, false, true, true, false, false, false>;
using SdLoDaFsa = Cdawg<true, false, false, true, false, false, false, false>;
using SdLoCnDaFsa = Cdawg<true, false, false, true, false, true, false, false>;
using SdLoCidDaFsa = Cdawg<true, false, true, true, false, false, false, false>;
using SdLoCsidDaFsa = Cdawg<true, false, true, true, false, false, true, false>;
using SdLoCnsidDaFsa = Cdawg<true, false, true, true, false, true, true, false>;
using SdLoDwDaFsa = Cdawg<true, false, false, true, false, false, false, true>;

}

#endif /* array_fsa_hpp */

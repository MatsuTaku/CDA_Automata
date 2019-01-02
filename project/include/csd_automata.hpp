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

using DaFsa = Dawg<false>;
using DaFsaDac = Dawg<true>;

// Recomended
using Daram = Cdawg<true, true, false, false, true, false, false, false, false>;

// For Expression
using SdDaFsa = Cdawg<true, true, true, false, true, true, false, false, false>;
using SdLoDaFsa = Cdawg<true, true, false, false, true, false, false, false, false>;
using SdLoCnDaFsa = Cdawg<true, true, false, false, true, false, true, false, false>;
using SdLoCidDaFsa = Cdawg<true, true, false, true, true, false, false, false, false>;
using SdLoCsidDaFsa = Cdawg<true, true, false, true, true, false, false, true, false>;
using SdLoCnsidDaFsa = Cdawg<true, true, false, true, true, false, true, true, false>;
using SdLoDwDaFsa = Cdawg<true, true, false, false, true, false, false, false, true>;
// Rejected
using SdLoSiDaFsa = Cdawg<false, true, false, true, true, false, false, false, false>;

}

#endif /* array_fsa_hpp */

//
//  NextCheck.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/20.
//

#include "NextCheck.hpp"

using namespace array_fsa;

template<> class NextCheck<DACs>;
template<> class NextCheck<SACs>;

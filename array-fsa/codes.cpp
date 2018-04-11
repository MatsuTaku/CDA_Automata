//
//  codes.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/02/06.
//

#include "CodesMeasurer.hpp"

using namespace array_fsa;

int main() {
    CodesMeasurer::bench<DACs<>>();
    CodesMeasurer::bench<SACs>();
    
}

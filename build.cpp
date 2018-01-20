#include <iostream>
#include <unistd.h>

#include "FsaGenerator.hpp"

//#include "ArrayFSA.hpp"
//#include "NArrayFSA.hpp"
//#include "NArrayFSADACs.hpp"
//#include "NArrayFSATextEdge.hpp"
#include "FSA.hpp"
#include "StringTransFSA.hpp"

using namespace array_fsa;

int main(int argc, const char *argv[]) {
    auto data_name = argv[1];
    auto fsa_name = argv[2];
    auto fsa_type = *argv[3];
    
////    data_name = "../../data-sets/weiss/wikipedia.dict";
////    fsa_name = "../../results/wikipedia/wikipedia.fsa";
//    data_name = "../../data-sets/kanda/indochina-2004.dict";
//    fsa_name = "../../results/indochina-2004/indochina-2004.fsa";
//    fsa_type = '5';

    switch (fsa_type) {
//        case '0':
//            return FsaGenerator::buildFSA<ArrayFSA>(data_name, fsa_name);
//        case '1':
//            return FsaGenerator::buildFSA<NArrayFSA>(data_name, fsa_name);
//        case '2':
//            return FsaGenerator::buildFSA<NArrayFSADACs>(data_name, fsa_name);
//        case '3':
//            return FsaGenerator::buildFSA<NArrayFSATextEdge>(data_name, fsa_name);
        case '4':
            return FsaGenerator<OriginalFSA>::buildFSA(data_name, fsa_name);
        case '5':
            return FsaGenerator<DacFSA>::buildFSA(data_name, fsa_name);
        case '6':
            return FsaGenerator<STFSA>::buildFSA(data_name, fsa_name);
        case '7':
            return FsaGenerator<DacSTFSA>::buildFSA(data_name, fsa_name);
        default:
            return 1;
    }
    
}

#include <iostream>
#include <unistd.h>

#include "FsaGenerator.hpp"

#include "ArrayFSA.hpp"
#include "NArrayFSA.hpp"
#include "NArrayFSADACs.hpp"
#include "NArrayFSATextEdge.hpp"

using namespace array_fsa;

int main(int argc, const char *argv[]) {
    auto data_name = argv[1];
    auto fsa_name = argv[2];
    auto fsa_type = *argv[3];
    
//    data_name = "../../data-sets/weiss/wikipedia.dict";
//    fsa_name = "../../results/wikipedia/wikipedia.n_array_fsa_dacs";
//    data_name = "../../data-sets/kanda/indochina-2004.dict";
//    fsa_name = "../../results/indochina-2004/indochina-2004.n_array_fsa_dacs";
//    fsa_type = '2';

    switch (fsa_type) {
        case '0':
            return FsaGenerator<ArrayFSA>::buildFSA(data_name, fsa_name);
        case '1':
            return FsaGenerator<NArrayFSA>::buildFSA(data_name, fsa_name);
        case '2':
            return FsaGenerator<NArrayFSADACs>::buildFSA(data_name, fsa_name);
        case '3':
            return FsaGenerator<NArrayFSATextEdge>::buildFSA(data_name, fsa_name);
        default:
            return 1;
    }
    
}

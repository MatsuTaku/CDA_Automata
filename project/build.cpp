#include <iostream>
#include <unistd.h>

#include "array_fsa.hpp"
#include "FsaGenerator.hpp"

using namespace array_fsa;

int main(int argc, const char *argv[]) {
    auto data_name = argv[1];
    auto plain_fsa_name = argv[2];
    auto fsa_name = argv[3];
    auto fsa_type = *argv[4];
    
//    data_name = "../../../data-sets/weiss/wikipedia.dict";
//    plain_fsa_name = "../../../results/wikipedia/wikipedia.plain";
//    fsa_name = "../../../results/wikipedia/wikipedia.fsa";
//    data_name = "../../../data-sets/weiss/wikipedia2.dict";
//    plain_fsa_name = "../../../results/wikipedia2/wikipedia2.plain";
//    fsa_name = "../../../results/wikipedia2/wikipedia2.sample";
//    data_name = "../../../data-sets/kanda/indochina-2004.dict";
//    plain_fsa_name = "../../../results/indochina-2004/indochina-2004.plain";
//    fsa_name = "../../../results/indochina-2004/indochina-2004.sample"; fsa_type = '3';
//    plain_fsa_name = "../../../results/jawiki-20150118/jawiki-20150118.plain";
//    data_name = "../../../data-sets/kanda/jawiki-20150118.dict";
//    fsa_name = "../../../results/jawiki-20150118/jawiki-20150118.sample"; fsa_type = '3';
//    plain_fsa_name = "../../../results/abc/abc.plain";
//    data_name = "../../../data-sets/ciura-deorowicz/abc.dict";
//    fsa_name = "../../../results/abc/abc.sample"; fsa_type = '3';
//    fsa_type = '3';
//    plain_fsa_name = "../../../results/enable/enable.plain";
//    data_name = "../../../data-sets/ciura-deorowicz/enable.dict";
//    fsa_name = "../../../results/enable/enable.sample"; fsa_type = '3';
//    data_name = "../../../data-sets/ciura-deorowicz/full.dict";
//    plain_fsa_name = "../../../results/full/full.plain";
//    fsa_name = "../../../results/full/full.fsa";
//    fsa_type = '3';
    
    switch (fsa_type) {
        case '1':
            return FsaGenerator<OriginalFSA>::buildFSA(data_name, plain_fsa_name, fsa_name);
        case '2':
            return FsaGenerator<DacFSA>::buildFSA(data_name, plain_fsa_name, fsa_name);
        case '3':
            return FsaGenerator<DoubleArrayCFSA<true, true, true, true>>::buildFSA(data_name, plain_fsa_name, fsa_name);
        case '4':
            return FsaGenerator<DoubleArrayCFSA<false, true, true, true>>::buildFSA(data_name, plain_fsa_name, fsa_name);
        case '5':
            return FsaGenerator<DoubleArrayCFSA<true, false, true, true>>::buildFSA(data_name, plain_fsa_name, fsa_name);
        case '6':
            return FsaGenerator<DoubleArrayCFSA<true, true, false, true>>::buildFSA(data_name, plain_fsa_name, fsa_name);
        case '7':
            return FsaGenerator<DoubleArrayCFSA<true, true, true, false>>::buildFSA(data_name, plain_fsa_name, fsa_name);
        default:
            std::cout << "Error: fsa_type is not set!" << std::endl;
            return 1;
    }
    
}
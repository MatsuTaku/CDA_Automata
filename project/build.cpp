#include <iostream>
#include <unistd.h>

#include "csd_automata.hpp"
#include "DABuilder.hpp"
#include "DartsCloneWrapper.hpp"

using namespace csd_automata;

namespace {
    
    template <class FSA_TYPE>
    int buildDoubleArray(const char *dataName, const char *plainFsaName, const char *fsaName) {
        DABuilder<FSA_TYPE> builder;
        std::cout << "Build Double-Array Automata from " << plainFsaName << std::endl;
        try {
            builder.buildFromFSA(plainFsaName);
        } catch (exception::DataNotFound e) {
            std::cerr << "Error open: " << e.data_name_ << std::endl;
            return -1;
        }
        
        std::cout << "Test for membership" << std::endl;
        try {
            builder.checkHasMember(dataName);
        } catch (exception::DoesntHaveMember e) {
            std::cout << "Doesn't have member: " << e.text << std::endl;
            return -1;
        } catch (exception::LookupError e) {
            e.error();
            return -1;
        } catch (exception::AccessError e) {
            e.error();
            return -1;
        }
        
        std::cout << "Write into " << fsaName << std::endl;
        builder.save(fsaName);
        
        return 0;
    }
    
    int buildDarts(const char *dataName, const char *outName) {
        wrapper::DartsCloneWrapper da;
        
        std::cout << "Build darts-clone from " << dataName << std::endl;
        da.build(dataName);
        
        std::cout << "Write into " << outName << std::endl;
        return da.write(outName);
    }
    
}

int main(int argc, const char *argv[]) {
    auto data_name = argv[1];
    auto plain_fsa_name = argv[2];
    auto fsa_name = argv[3];
    auto fsa_type = atoi(argv[4]);
    
//    data_name = "../../../data-sets/weiss/wikipedia.dict";
//    plain_fsa_name = "../../../results/wikipedia/wikipedia.plain";
//    fsa_name = "../../../results/wikipedia/wikipedia.da";
//    fsa_type = 9;
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
        case 1:
            return buildDoubleArray<OriginalFSA>(data_name, plain_fsa_name, fsa_name);
        case 2:
            return buildDoubleArray<DacFSA>(data_name, plain_fsa_name, fsa_name);
        case 3:
            return buildDoubleArray<DoubleArrayAutomataDictionary>(data_name, plain_fsa_name, fsa_name);
        case 4:
            return buildDoubleArray<DoubleArrayCFSA<false, true, true, true, true>>(data_name, plain_fsa_name, fsa_name);
        case 5:
            return buildDoubleArray<DoubleArrayCFSA<true, false, true, true, true>>(data_name, plain_fsa_name, fsa_name);
        case 6:
            return buildDoubleArray<DoubleArrayCFSA<true, true, false, true, true>>(data_name, plain_fsa_name, fsa_name);
        case 7:
            return buildDoubleArray<DoubleArrayCFSA<true, true, true, false, true>>(data_name, plain_fsa_name, fsa_name);
        case 8:
            return buildDoubleArray<DoubleArrayAutomataLookupDictionary>(data_name, plain_fsa_name, fsa_name);
        case 9:
            return buildDarts(data_name, fsa_name);
        default:
            std::cout << "Error: fsa_type is not set!" << std::endl;
            return 1;
    }
    
}

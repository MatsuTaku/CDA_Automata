#include <iostream>
#include <unistd.h>

#include "FsaGenerator.hpp"
#include "Exception.hpp"

#include "ArrayFSA.hpp"
#include "NArrayFSA.hpp"
#include "NArrayFSADACs.hpp"
#include "NArrayFSATextEdge.hpp"

using namespace array_fsa;

namespace {
    
    template <class T>
    int buildFSA(const char *dataName, const char *fsaName) {
        try {
            T fsa = FsaGenerator::generateFSA<T>(dataName);
            
            std::cout << "Test for membership" << std::endl;
            FsaGenerator::checkHasMember(fsa, dataName);
            
            std::cout << "Write FSA into " << fsaName << std::endl;
            FsaGenerator::saveFSA(fsa, fsaName);
            
        } catch (DataNotFoundException e) {
            std::cerr << "Error open " << e.data_name_ << std::endl;
            return 1;
        } catch (DoesntHaveMemberException e) {
            std::cout << "Doesn't have member: " << e.text << std::endl;
            return 1;
        }
        return 0;
    }
    
}

int main(int argc, const char *argv[]) {
    auto data_name = argv[1];
    auto fsa_name = argv[2];
    auto fsa_type = *argv[3];
    
    data_name = "../../data-sets/weiss/wikipedia.dict";
    fsa_name = "../../results/wikipedia/wikipedia.n_array_fsa";
    fsa_type = '1';
    
    std::cout << "Build FSA from " << data_name << std::endl;
    
    switch (fsa_type) {
        case '0':
            return buildFSA<ArrayFSA>(data_name, fsa_name);
        case '1':
            return buildFSA<NArrayFSA>(data_name, fsa_name);
        case '2':
            return buildFSA<NArrayFSADACs>(data_name, fsa_name);
        case '3':
            return buildFSA<NArrayFSATextEdge>(data_name, fsa_name);
        default:
            return 1;
    }
    
}

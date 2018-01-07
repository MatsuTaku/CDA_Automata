//
// Created by Kampersanda on 2017/05/29.
//

#include <iostream>

#include "FsaTools.hpp"
#include "MorfologikFSA5.hpp"
#include "MorfologikCFSA2.hpp"
#include "ArrayFSA.hpp"
#include "ArrayDACFSA.hpp"
#include "ArrayFSATail.hpp"
#include "ArrayFSATailDAC.hpp"
#include "NArrayFSA.hpp"
#include "NArrayFSADACs.hpp"
#include "NArrayFSATextEdge.hpp"

using namespace array_fsa;

namespace {
    
    template <typename T>
    int bench(const char* fsa_name, const char* query_name) {
        try {
            std::cout << "Bench " << T::name() << " from " << fsa_name << std::endl;
            
            T fsa = FsaTools::getFSAFrom<T>(fsa_name);
            
            std::cout << "Lookup bench for " << query_name << std::endl;
            
            FsaTools::measureBenchmark(fsa, query_name);
            
            return 0;
        } catch (DataNotFoundException e) {
            std::cerr << "Error open " << e.data_name_ << std::endl;
            return 1;
        } catch (DoesntHaveMemberExceptipn e) {
            std::cout << "Doesn't have member: " << e.text << std::endl;
            return 1;
        }
    }
    
}

int main(int argc, const char* argv[]) {
    auto fsa_name = argv[1];
    auto query_name = argv[2];
    auto type = *argv[3];
    
//    fsa_name = "../../results/wikipedia/wikipedia.n_array_fsa_text";
//    query_name = "../../data-sets/weiss/wikipedia.1000000.rnd_dict";
//    type = '8';
    
    switch (type) {
        case '0':
            return bench<ArrayFSA>(fsa_name, query_name);
        case '1':
            return bench<MorfologikFSA5>(fsa_name, query_name);
        case '2':
            return bench<MorfologikCFSA2>(fsa_name, query_name);
        case '3':
            return bench<ArrayDACFSA>(fsa_name, query_name);
        case '4':
            return bench<ArrayFSATail>(fsa_name, query_name);
        case '5':
            return bench<ArrayFSATailDAC>(fsa_name, query_name);
        case '6':
            return bench<NArrayFSA>(fsa_name, query_name);
        case '7':
            return bench<NArrayFSADACs>(fsa_name, query_name);
        case '8':
            return bench<NArrayFSATextEdge>(fsa_name, query_name);
        default:
            break;
    }
    
    return 1;
}

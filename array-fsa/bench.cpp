//
// Created by Kampersanda on 2017/05/29.
//

#include "array_fsa.hpp"
#include "FsaTools.hpp"

using namespace array_fsa;

namespace {
    
    template <typename T>
    int bench(const char* fsa_name, const char* query_name) {
        try {
            std::cout << "Bench " << T::name() << " from " << fsa_name << std::endl;
            
            T fsa = FsaTools::getFSAFrom<T>(fsa_name);
            
            std::cout << "Lookup bench for " << query_name << std::endl;
            
            FsaTools::measureBenchmark(fsa, query_name);
            
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

int main(int argc, const char* argv[]) {
    auto fsa_name = argv[1];
    auto query_name = argv[2];
    auto type = *argv[3];
    
//    fsa_name = "../../results/wikipedia/wikipedia.fsa";
//    query_name = "../../data-sets/weiss/wikipedia.1000000.rnd_dict";
//    fsa_name = "../../results/indochina-2004/indochina-2004.sac_array_fsa"; type = '6';
//    fsa_name = "../../results/indochina-2004/indochina-2004.stcfsa"; type = '7';
//    query_name = "../../data-sets/kanda/indochina-2004.1000000.rnd_dict";
//    fsa_name = "../../results/jawiki-20150118/jawiki-20150118.fsa"; type = '6';
//    fsa_name = "../../results/jawiki-20150118/jawiki-20150118.stcfsa";type = '7';
//    query_name = "../../data-sets/kanda/jawiki-20150118.1000000.rnd_dict";
//    type = '3';
    
    switch (type) {
        case '1':
            return bench<MorfologikFSA5>(fsa_name, query_name);
        case '2':
            return bench<MorfologikCFSA2>(fsa_name, query_name);
        case '3':
            return bench<OriginalFSA>(fsa_name, query_name);
        case '4':
            return bench<DacFSA>(fsa_name, query_name);
        case '5':
            return bench<STFSA>(fsa_name, query_name);
        default:
            return 1;
    }

}

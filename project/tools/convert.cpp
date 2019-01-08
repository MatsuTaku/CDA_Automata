// ArrayFSA
//  convert.cpp
//  ArrayFSA_dev
//
//  Created by 松本拓真 on 2018/09/24.
//

#include "csd_automata/MorfologikFsaDictionary.hpp"

using namespace csd_automata;

namespace {
    
    template<class Dictionary>
    void BuildMorfologikFSADictionary(const char *setName, const char *dictName) {
        std::ifstream ifs(setName);
        if (!ifs) {
            std::cerr << "Error! File not found: " << setName << std::endl;
            std::exit(EXIT_FAILURE);
        }
        typename Dictionary::FsaSource fsa(ifs);
        Dictionary mf5d(fsa);
        StoreToFile(mf5d, dictName);
    }
    
}

int main(int argc, const char* argv[]) {
    auto set_name = argv[1];
    auto dict_name = argv[2];
    int set_type = atoi(argv[3]);
    
#ifndef NDEBUG
    set_name = "../../../../results/wikipedia2/wikipedia2.morfologik_cfsa2";
    dict_name = "../../../../results/wikipedia2/wikipedia2.morfologik_cfsa2d";
    set_type = 1;
#endif
    
    switch (set_type) {
        case 0:
            BuildMorfologikFSADictionary<SdMrfFsa5>(set_name, dict_name);
            break;
        case 1:
            BuildMorfologikFSADictionary<SdMrfCFsa2>(set_name, dict_name);
            break;
        default:
            std::cerr << "Error setType: " << set_type << std::endl;
            return -1;
    }
    
    return 0;
}

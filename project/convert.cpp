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
    void buildMorfologikFSADictionary(const char *setName, const char *dictName) {
        std::ifstream ifs(setName);
        if (!ifs) {
            std::cerr << "Error! File not found: " << setName << std::endl;
        }
        typename Dictionary::Foundation::FsaSource fsa;
        fsa.read(ifs);
        typename Dictionary::Foundation foundation(fsa);
        Dictionary mf5d(std::move(foundation));
        std::ofstream ofs(dictName);
        mf5d.Write(ofs);
    }
    
}

int main(int argc, const char* argv[]) {
    auto setName = argv[1];
    auto dictName = argv[2];
    int setType = atoi(argv[3]);
    
#ifndef NDEBUG
    setName = "../../../results/enwiki-20150205/enwiki-20150205.morfologik_cfsa2";
    dictName = "../../../results/enwiki-20150205/enwiki-20150205.morfologik_cfsa2";
    setType = 1;
#endif
    
    switch (setType) {
        case 0:
            buildMorfologikFSADictionary<SdMrfFsa5>(setName, dictName);
            break;
        case 1:
            buildMorfologikFSADictionary<SdMrfCfsa2>(setName, dictName);
            break;
        default:
            std::cerr << "Error setType: " << setType << std::endl;
            return -1;
    }
    
    return 0;
}

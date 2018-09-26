//
//  convert.cpp
//  ArrayFSA_dev
//
//  Created by 松本拓真 on 2018/09/24.
//

#include "array_fsa/MorfologikFSADictionary.hpp"
#include "array_fsa/MorfologikCFSA2.hpp"

using namespace array_fsa;

namespace {
    
    void buildMorfologikFSA5Dictionary(const char *setName, const char *dictName) {
        std::ifstream ifs(setName);
        if (!ifs) {
            std::cerr << "Error! File not found: " << setName << std::endl;
        }
        
        MorfologikFSA5 mf5;
        mf5.read(ifs);
        
        MorfologikFSADictionary mf5d(mf5);
        std::ofstream ofs(dictName);
        mf5d.write(ofs);
    }
    
}

int main(int argc, const char* argv[]) {
    auto setName = argv[1];
    auto dictName = argv[2];
    auto setType = atoi(argv[3]);
    
//    setName = "../../../results/wikipedia/wikipedia.morfologik_fsa5";
//    dictName = "../../../results/wikipedia/wikipedia.morfologik_fsa5";
//    setType = 0;
    
    switch (setType) {
        case 0:
            buildMorfologikFSA5Dictionary(setName, dictName);
            break;
        case 1:
            break;
        default:
            std::cerr << "Error setType: " << setType << std::endl;
            return -1;
    }
    
    return 0;
}

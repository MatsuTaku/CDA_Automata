//
//  build.cpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/10/01.
//

#include "csd_automata.hpp"
#include "csd_automata/Director.hpp"

int main(int argc, const char *argv[]) {
#ifdef NDEBUG
    if (argc < 3) {
        std::cerr << "Invalid number of arguments!" << std::endl;
        return -1;
    }
#endif
    
    auto datasetName = argv[1];
    auto dictName = argv[2];
    
#ifndef NDEBUG
    datasetName = "../../../data-sets/weiss/wikipedia.dict";
    dictName = "../../../results/wikipedia/wikipedia.dam";
#endif
    
    using types = csd_automata::DAMTypes;
    
    int typeIndex = 0;
    for (int i = 2; i < argc; i++) {
        if (std::string(argv[i]) == "--access")
            typeIndex = 1;
    }
    
    switch (typeIndex) {
        case 0:
            return csd_automata::director::fullyBuild<std::tuple_element_t<0, types>>(datasetName, dictName);
        case 1:
            return csd_automata::director::fullyBuild<std::tuple_element_t<1, types>>(datasetName, dictName);
        default:
            break;
    }
    
    return -1;
    
}

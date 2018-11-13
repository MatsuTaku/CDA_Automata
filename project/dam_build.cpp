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
    
    int typeIndex = 0;
    std::string valuesName = "";
    
    for (int i = 2; i < argc; i++) {
        std::string option(argv[i]);
        if (option == "--access") {
            typeIndex = 1;
        } else if (option == "--values") {
            valuesName = argv[i + 1];
            i++;
        }
    }
    
#ifndef NDEBUG
    datasetName = "../../data-sets/local/enwiki-20181001.dict";
    dictName = "../../results/enwiki-20181001/enwiki-20181001.dam";
    typeIndex = 0;
#endif
    
    using types = csd_automata::DAMTypes;
    switch (typeIndex) {
        case 0:
            return csd_automata::director::fullyBuild<std::tuple_element_t<0, types>>(dictName, datasetName, valuesName);
        case 1:
            return csd_automata::director::fullyBuild<std::tuple_element_t<1, types>>(dictName, datasetName, valuesName);
        default:
            break;
    }
    
    return -1;
    
}

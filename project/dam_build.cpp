//
//  build.cpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/10/01.
//

#include "csd_automata.hpp"
#include "csd_automata/Director.hpp"

using namespace csd_automata;

int main(int argc, const char* argv[]) {
#ifdef NDEBUG
    if (argc < 3) {
        std::cerr << "Invalid number of arguments!" << std::endl;
        return -1;
    }
#endif
    
    auto dataset_name = argv[1];
    auto dict_name = argv[2];
    
    int type_index = 0;
    std::string valuesName = "";
    
    for (int i = 2; i < argc; i++) {
        std::string option(argv[i]);
        if (option == "--access") {
            type_index = 1;
        } else if (option == "--values") {
            valuesName = argv[i + 1];
            i++;
        }
    }
    
#ifndef NDEBUG
    dataset_name = "../../data-sets/local/jawiki-20181001.dict";
    dict_name = "../../results/jawiki-20181001/jawiki-20181001.dam";
    type_index = 0;
#endif
    
    using types = std::tuple<
    SdLoDaFsa,
    SdDaFsa
    >;
    
    switch (type_index) {
        case 0:
            return director::FullyBuild<std::tuple_element_t<0, types>>(dict_name, dataset_name, valuesName);
        case 1:
            return director::FullyBuild<std::tuple_element_t<1, types>>(dict_name, dataset_name, valuesName);
        default:
            break;
    }
    
    return -1;
    
}

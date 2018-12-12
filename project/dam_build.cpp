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
    std::string values_name = "";
    
    for (int i = 3; i < argc; i++) {
        std::string option(argv[i]);
        if (option == "--access") {
            type_index = 1;
        } else if (option == "--comp-id") {
            type_index = 2;
        } else if (option == "--values") {
            values_name = argv[i + 1];
            i++;
        }
    }
    
#ifndef NDEBUG
//    dataset_name = "../../data-sets/local/jawiki-20181001.dict";
    dataset_name = "../../data-sets/local/jawiki-20181001.1000000.rnd_dict";
    dict_name = "../../results/jawiki-20181001/jawiki-20181001.dam";
//    dataset_name = "../../data-sets/ciura-deorowicz/abc.dict";
//    dict_name = "../../results/abc/abc.dam";
    type_index = 2;
#endif
    
    switch (type_index) {
        case 0:
            return director::FullyBuild<SdLoDaFsa>(dict_name, dataset_name, values_name);
        case 2:
            return director::FullyBuild<SdLoCidDaFsa>(dict_name, dataset_name, values_name);
        case 1:
            return director::FullyBuild<SdDaFsa>(dict_name, dataset_name, values_name);
        case 3: // Rejected
            return director::FullyBuild<SdLoSiDaFsa>(dict_name, dataset_name, values_name);
        default:
            break;
    }
    
    return -1;
    
}

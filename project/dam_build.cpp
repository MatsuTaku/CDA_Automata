//
//  build.cpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/10/01.
//

#include "csd_automata.hpp"
#include "csd_automata/Director.hpp"

using namespace csd_automata;

namespace {

void ShowUsage() {
    std::cout << "Usage:"
    << "\t$ dam_build [DATASET] (-o | --output) [OUTPUT]" << std::endl;
}
    
}

int main(int argc, const char* argv[]) {
#ifdef NDEBUG
    if (argc < 3) {
        std::cerr << "Invalid number of arguments!" << std::endl;
        return -1;
    }
#endif
    
    std::string dataset_name = "";
    std::string dict_name = "";
    
    int type_index = 0;
    std::string values_name = "";
    
    for (int i = 1; i < argc; i++) {
        std::string option(argv[i]);
        if (option == "-o" || option == "--output") {
            dict_name = argv[++i];
        } else if (option == "--access") {
            type_index = 1;
        } else if (option == "--comp-id") {
            type_index = 2;
        } else if (option == "--comp-next") {
            type_index = 4;
        } else if (option == "--values") {
            values_name = argv[++i];
        } else {
            dataset_name = argv[i];
        }
    }
    
#ifndef NDEBUG
//    dataset_name = "../../data-sets/weiss/wikipedia2.dict";
//    dict_name = "../../results/wikipedia2/wikipedia2.dam";
//    dataset_name = "../../data-sets/local/enwiki-20181001.dict";
//    dict_name = "../../results/enwiki-20181001/enwiki-20181001.dam";
    dataset_name = "../../data-sets/local/jawiki-20181001.dict";
    dict_name = "../../results/jawiki-20181001/jawiki-20181001.dam";
//    dataset_name = "../../data-sets/ciura-deorowicz/abc.dict";
//    dict_name = "../../results/abc/abc.dam";
    type_index = 0;
#endif
    
    if (dataset_name == "") {
        std::cout << "Dataset target not found!" << std::endl;
        ShowUsage();
        return -1;
    }
    if (dict_name == "") {
        std::cout << "option '(-o | --output) [OUTPUT]' is required!" << std::endl;
        ShowUsage();
        return -1;
    }
    
    switch (type_index) {
        case 0: // Recomended
            return director::FullyBuild<SdLoDaFsa>(dict_name, dataset_name, values_name);
        case 1: // supporting ACCESS
            return director::FullyBuild<SdDaFsa>(dict_name, dataset_name, values_name);
        case 2: // ! Not recomended
            return director::FullyBuild<SdLoCidDaFsa>(dict_name, dataset_name, values_name);
        case 4: // ! Not recomended
            return director::FullyBuild<SdLoDacDaFsa>(dict_name, dataset_name, values_name);
        case 3: // ! Rejected
            return director::FullyBuild<SdLoSiDaFsa>(dict_name, dataset_name, values_name);
        default:
            break;
    }
    
    return -1;
    
}

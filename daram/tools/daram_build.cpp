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

struct InputOption {
    uint8_t binary = 0;
    
    void access() {
        binary |= 0b01;
    }
    void comp_id() {
        binary |= 0b10;
    }
    void select_id() {
        binary |= 0b100;
    }
    void comp_next() {
        binary |= 0b1000;
    }
    void dac_cwords() {
        binary |= 0b10000;
    }
};

}

int main(int argc, const char* argv[]) {
    std::string dataset_name = "";
    std::string dict_name = "";
    InputOption options;
    std::string values_name = "";
    
#ifdef NDEBUG
    if (argc < 3) {
        std::cerr << "Invalid number of arguments!" << std::endl;
        return -1;
    }
    for (int i = 1; i < argc; i++) {
        std::string option(argv[i]);
        if (option == "-o" or option == "--output") {
            dict_name = argv[++i];
        } else if (option == "--access") {
            options.access();
        } else if (option == "--comp-id") {
            options.comp_id();
        } else if (option == "--select-id") {
            options.select_id();
        } else if (option == "--comp-next") {
            options.comp_next();
        } else if (option == "--dac-cwords") {
            options.dac_cwords();
        } else if (option == "--values") {
            values_name = argv[++i];
        } else {
            dataset_name = argv[i];
        }
    }
    
#else
//    dataset_name = "../../../../data-sets/weiss/wikipedia2.dict";
//    dict_name = "../../../../results/wikipedia2/wikipedia2.dam";
//    dataset_name = "../../../../data-sets/local/enwiki-20181001.dict";
//    dict_name = "../../../../results/enwiki-20181001/enwiki-20181001.dam";
    dataset_name = "../../../../data-sets/local/jawiki-20181001.dict";
    dict_name = "../../../../results/jawiki-20181001/jawiki-20181001.dam";
//    dataset_name = "../../../../data-sets/ciura-deorowicz/full.dict";
//    dict_name = "../../../../results/full/full.dam";
    options.binary = 0b00000;
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
    
    switch (options.binary) {
        case 0b00000: // Recomended
            return director::FullyBuild<SdLoDaFsa>(dict_name, dataset_name, values_name);
        case 0b00010: // Comparations
            return director::FullyBuild<SdLoCidDaFsa>(dict_name, dataset_name, values_name);
        case 0b00110: // Comparations
            return director::FullyBuild<SdLoCsidDaFsa>(dict_name, dataset_name, values_name);
        case 0b01110: // Comparations
            return director::FullyBuild<SdLoCnsidDaFsa>(dict_name, dataset_name, values_name);
        case 0b01000: // Comparations
            return director::FullyBuild<SdLoCnDaFsa>(dict_name, dataset_name, values_name);
        case 0b10000: // Comparations
            return director::FullyBuild<SdLoDwDaFsa>(dict_name, dataset_name, values_name);
        case 0b00001: // supporting ACCESS
            return director::FullyBuild<SdDaFsa>(dict_name, dataset_name, values_name);
        default:
            break;
    }
    
    return -1;
    
}

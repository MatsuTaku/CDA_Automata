//
//  samc_build.cpp
//  darts_build
//
//  Created by 松本拓真 on 2019/05/27.
//

#include "SamcWrapper.hpp"
#include <iostream>

int main(int argc, const char* argv[]) {
    auto dataset_name = argv[1];
    auto output_name = argv[2];
    
#ifndef NDEBUG
    dataset_name = "../../../data-sets/ciura-deorowicz/deutsch.dict";
    output_name = "../../../results/deutsch/deutsch.samc";
//    dataset_name = "../../../data-sets/local/jawiki-20181001.dict";
//    output_name = "../../../results/jawiki-20181001/jawiki-20181001.samc";
//    dataset_name = "../../../data-sets/weiss/wikipedia.dict";
//    output_name = "../../../results/wikipedia/wikipedia.samc";
#endif
    
    std::cout << "Build samc from: " << dataset_name << std::endl;
    std::ifstream ifs(dataset_name);
    if (!ifs) {
        std::cout << "Not found file: " << dataset_name << std::endl;
        std::exit(EXIT_FAILURE);
    }
    wrapper::SamcWrapper samc(ifs);
    std::cout << "Output: " << output_name << std::endl;
    auto ofs = std::ofstream(output_name);
    samc.Write(ofs);
    
    return 0;
}

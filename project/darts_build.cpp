//
//  darts_build.cpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/10/05.
//

#include "DartsCloneWrapper.hpp"

int main(int argc, const char* argv[]) {
    auto dataset_name = argv[1];
    auto output_name = argv[2];
    
#ifndef NDEBUG
    dataset_name = "../../../data-sets/weiss/wikipedia.dict";
    output_name = "../../../results/wikipedia/wikipedia.darts";
#endif
    
    std::cout << "Build darts from: " << dataset_name << std::endl;
    wrapper::DartsCloneWrapper darts;
    darts.Build(dataset_name);
    std::cout << "Output: " << output_name << std::endl;
    return darts.Write(output_name);
}

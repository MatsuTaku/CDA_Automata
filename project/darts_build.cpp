//
//  darts_build.cpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/10/05.
//

#include "DartsCloneWrapper.hpp"

int main(int argc, const char* argv[]) {
    auto datasetName = argv[1];
    auto outputName = argv[2];
    
#ifndef NDEBUG
    datasetName = "../../../data-sets/weiss/wikipedia.dict";
    outputName = "../../../results/wikipedia/wikipedia.darts";
#endif
    
    std::cout << "Build darts from: " << datasetName << std::endl;
    wrapper::DartsCloneWrapper darts;
    darts.build(datasetName);
    std::cout << "Output: " << outputName << std::endl;
    return darts.write(outputName);
}

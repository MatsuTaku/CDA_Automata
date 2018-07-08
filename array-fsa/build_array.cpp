//
//  build_array.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/06/06.
//

#include <stdio.h>
#include "array_fsa/StringTransFSA.hpp"

using namespace array_fsa;

namespace {
    
    void buildCheck(const char* dataName, const char* arrayName) {
        std::cout << "Build array_check from: " << dataName << std::endl;
        std::ifstream ifs(dataName);
        if (!ifs) {
            std::cout << "File not found: " << dataName << std::endl;
        }
        STFSA fsa;
        fsa.read(ifs);
        std::ofstream ofs(arrayName);
        std::cout << "...written: " << arrayName << std::endl;
        fsa.writeCheck(ofs);
    }
    
}

int main(int argc, char* argv[]) {
    const char* data_name = argv[1];
    const char* array_name = argv[2];
    
//    data_name = "../../results/wikipedia/wikipedia.array_ts_fsa";
//    array_name = "../../results/wikipedia/wikipedia.arr_check";
    
    buildCheck(data_name, array_name);
    
    return 0;
}

//
//  makefsa.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/03/20.
//

#include "array_fsa/basic.hpp"
#include "array_fsa/PlainFSABuilder.hpp"
#include "FsaGenerator.hpp"

using namespace array_fsa;


int main(int argc, const char* argv[]) {
    auto dataName = argv[1];
    auto plainFsaName = argv[2];
//    dataName = "../../../data-sets/ciura-deorowicz/abc.dict";
//    plainFsaName = "../../../results/abc/abc.plain";
//    dataName = "../../../data-sets/kanda/jawiki-20150118.dict";
//    plainFsaName = "../../../results/jawiki-20150118/jawiki-20150118.plain";
    
    PlainFsaGenerator::generate(dataName, plainFsaName);
}

//
//  makefsa.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/03/20.
//

#include "basic.hpp"
#include "PlainFSABuilder.hpp"
#include "FsaGenerator.hpp"

using namespace array_fsa;

int main(int argc, const char* argv[]) {
    auto dataName = argv[1];
    auto plainFsaName = argv[2];
    PlainFsaGenerator::savePlainFsa(dataName, plainFsaName);
}
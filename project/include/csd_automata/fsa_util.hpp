//
//  fsa_util.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/12/30.
//

#ifndef fsa_util_hpp
#define fsa_util_hpp

#include "PlainFSA.hpp"
#include "PlainFSABuilder.hpp"
#include "util.hpp"

namespace csd_automata::fsa_util {

template <class Fsa>
bool SetFSAFromFile(const char* fsa_name, Fsa& fsa) {
    std::ifstream ifs(fsa_name);
    if (!ifs) {
        std::cout << "Not found FSA: " << fsa_name << std::endl;
        return false;
    }
    
    fsa = Fsa(ifs);
    return true;
}

// Might throw DataNotFoundException
inline PlainFSA BuildPlainFSA(const std::string& data_name) {
    auto ifs = util::GetStreamOrDie<std::ifstream>(data_name);
    
    PlainFSABuilder builder;
    for (std::string line; getline(ifs, line);)
        builder.add(line);
    
    return builder.release();
}

inline PlainFSA ReadPlainFSA(const std::string& plain_fsa_name) {
    auto ifs = util::GetStreamOrDie<std::ifstream>(plain_fsa_name);
    PlainFSA plainFsa;
    plainFsa.read(ifs);
    return plainFsa;
}

inline void Generate(const std::string& data_name, const std::string& plain_fsa_name) {
    auto plain_fsa = BuildPlainFSA(data_name);
    auto ofs = util::GetStreamOrDie<std::ofstream>(data_name);
    std::cout << "Write PlainFSA into " << plain_fsa_name << std::endl;
    plain_fsa.write(ofs);
}

}

#endif /* fsa_util_hpp */

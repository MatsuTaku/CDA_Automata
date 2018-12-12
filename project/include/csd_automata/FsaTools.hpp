//
// Created by Kampersanda on 2017/05/28.
//

#ifndef ARRAY_FSA_FSATOOLS_HPP
#define ARRAY_FSA_FSATOOLS_HPP

#include "csd_automata.hpp"

namespace csd_automata {

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
    
std::vector<std::string> GetKeySets(const char *queryName) {
    auto ifs = GetStreamOrDie<std::ifstream>(queryName);
    std::vector<std::string> strs;
    for (std::string line; std::getline(ifs, line);)
        strs.emplace_back(line);
    return strs;
}
    
}

#endif //ARRAY_FSA_FSATOOLS_HPP

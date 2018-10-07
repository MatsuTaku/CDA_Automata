//
// Created by Kampersanda on 2017/05/28.
//

#ifndef ARRAY_FSA_FSATOOLS_HPP
#define ARRAY_FSA_FSATOOLS_HPP

#include "csd_automata.hpp"
#include "csd_automata/Exception.hpp"

namespace csd_automata {
    
    namespace KeySet {
        
        std::vector<std::string> getKeySets(const char *queryName) {
            std::ifstream ifs(queryName);
            if (!ifs)
                throw exception::DataNotFound(queryName);
            
            std::vector<std::string> strs;
            for (std::string line; std::getline(ifs, line);)
                strs.emplace_back(line);
            return strs;
        }
        
    };
    
}

#endif //ARRAY_FSA_FSATOOLS_HPP
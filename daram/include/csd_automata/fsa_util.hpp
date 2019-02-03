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

// Might throw DataNotFoundException
inline PlainFSA make_plain_fsa(const std::string& keyset_name) {
    auto ifs = util::GetStreamOrDie<std::ifstream>(keyset_name);
    
    PlainFSABuilder builder;
    for (std::string line; getline(ifs, line);)
        builder.add(line);
    
    return builder.release();
}

inline PlainFSA read_plain_fsa(const std::string& plain_fsa_name) {
    auto ifs = util::GetStreamOrDie<std::ifstream>(plain_fsa_name);
    PlainFSA plainFsa;
    plainFsa.LoadFrom(ifs);
    return plainFsa;
}

inline void Generate(const std::string& data_name, const std::string& plain_fsa_name) {
    auto plain_fsa = make_plain_fsa(data_name);
    auto ofs = util::GetStreamOrDie<std::ofstream>(data_name);
    std::cout << "Write PlainFSA into " << plain_fsa_name << std::endl;
    plain_fsa.StoreTo(ofs);
}

}

#endif /* fsa_util_hpp */

//
//  MarisaWrapper.hpp
//  ArrayFSA_dev
//
//  Created by 松本拓真 on 2018/07/20.
//

#ifndef MarisaWrapper_hpp
#define MarisaWrapper_hpp

#include "csd_automata/basic.hpp"
#include "marisa/trie.h"

#include "csd_automata/Exception.hpp"

namespace wrapper {
    
void SetMarisaKeySet(const char* query_name, marisa::Keyset* keyset) {
    std::ifstream ifs(query_name);
    if (!ifs)
        throw csd_automata::exception::DataNotFound(query_name);
    
    for (std::string line; std::getline(ifs, line);) {
        std::string::size_type delim_pos = line.find_last_of('\t');
        float weight = 1.0F;
        if (delim_pos != line.npos) {
            char *end_of_value;
            weight = (float)std::strtod(&line[delim_pos + 1], &end_of_value);
            if (*end_of_value == '\0') {
                line.resize(delim_pos);
            }
        }
        keyset->push_back(line.c_str(), line.length());
    }
}

class MarisaWrapper {
    using Trie = marisa::Trie;
    using Agent = marisa::Agent;
    
public:
    
    static std::string name() {
        return typeid(Trie).name();
    }
    
    MarisaWrapper(const char* filename) {
        trie_.load(filename);
    }
    
    bool Accept(Agent& agent) const {
        return trie_.lookup(agent);
    }
    
    bool Lookup(Agent& agent) const {
        return trie_.lookup(agent);
    }
    
    void Access(Agent& agent) const {
        trie_.reverse_lookup(agent);
    }
    
    void ShowStatus(std::ostream& os) const {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "#tries:   " << trie_.num_tries() << endl;
        os << "#nodes:   " << trie_.num_nodes() << endl;
        os << "size:   " << trie_.size() << endl;
        os << "total_size:   " << trie_.total_size() << endl;
        os << "io_size:   " << trie_.io_size() << endl;
    }
    
private:
    Trie trie_;
    
};
    
}


#endif /* MarisaWrapper_hpp */

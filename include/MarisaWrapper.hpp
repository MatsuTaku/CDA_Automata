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

namespace wrapper {
    
void SetMarisaKeySet(std::ifstream& ifs, marisa::Keyset* keyset) {
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
    
    static std::string tag() {
        return "marisa";
    }
    
    MarisaWrapper() = default;
    
    friend void LoadFromFile(MarisaWrapper& self, const char* file_name);
    
    bool Accept(Agent& agent) const {
        return marisa_trie_.lookup(agent);
    }
    
    bool Lookup(Agent& agent) const {
        return marisa_trie_.lookup(agent);
    }
    
    void Access(Agent& agent) const {
        marisa_trie_.reverse_lookup(agent);
    }
    
    size_t size_in_bytes() const {
        return marisa_trie_.io_size();
    }
    
    void ShowStatus(std::ostream& os) const {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "#tries:   " << marisa_trie_.num_tries() << endl;
        os << "#nodes:   " << marisa_trie_.num_nodes() << endl;
        os << "size:   " << marisa_trie_.size() << endl;
        os << "total_size:   " << marisa_trie_.total_size() << endl;
        os << "io_size:   " << marisa_trie_.io_size() << endl;
    }
    
private:
    Trie marisa_trie_;
    
};

void LoadFromFile(MarisaWrapper& self, const char* file_name) {
    self.marisa_trie_.load(file_name);
}

}


#endif /* MarisaWrapper_hpp */

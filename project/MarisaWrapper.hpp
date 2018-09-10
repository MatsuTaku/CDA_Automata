//
//  MarisaWrapper.hpp
//  ArrayFSA_dev
//
//  Created by 松本拓真 on 2018/07/20.
//

#ifndef MarisaWrapper_hpp
#define MarisaWrapper_hpp

#include "marisa/trie.h"

namespace array_fsa {
    
    class MarisaWrapper {
    public:
        MarisaWrapper(const char* filename) {
            trie_.load(filename);
        }
        
    public:
        static std::string name() {
            return typeid(marisa::Trie).name();
        }
        
        bool isMember(marisa::Agent &agent) const {
            return trie_.lookup(agent);
        }
        
        bool lookup(marisa::Agent &agent) const {
            return trie_.lookup(agent);
        }
        
        void access(marisa::Agent &agent) const {
            trie_.reverse_lookup(agent);
        }
        
        void showStatus(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#tries:   " << trie_.num_tries() << endl;
            os << "#nodes:   " << trie_.num_nodes() << endl;
            os << "size:   " << trie_.size() << endl;
            os << "total_size:   " << trie_.total_size() << endl;
            os << "io_size:   " << trie_.io_size() << endl;
        }
        
    private:
        marisa::Trie trie_;
    };
}

#endif /* MarisaWrapper_hpp */

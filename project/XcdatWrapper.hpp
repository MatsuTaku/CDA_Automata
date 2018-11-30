//
//  XcdatWrapper.hpp
//  ArrayFSA_dev
//
//  Created by 松本拓真 on 2018/07/18.
//

#ifndef XcdatWrapper_hpp
#define XcdatWrapper_hpp

#include "xcdat/Trie.hpp"

namespace wrapper {
    
template<bool FAST>
class XcdatWrapper {
public:
    using xcdat_trie = xcdat::Trie<FAST>;
    
    xcdat_trie trie_;
    
    XcdatWrapper(std::istream& is) : trie_(is) {}
    
    static std::string name() {
        return typeid(xcdat_trie).name();
    }
    
public:
    
    bool isMember(const std::string &str) const {
        return trie_.lookup(str) != -1;
    }
    
    size_t lookup(const std::string &str) const {
        return trie_.lookup(str);
    }
    
    std::string access(size_t value) const {
        return trie_.access(value);
    }
    
    void ShowStatus(std::ostream& os) const {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "#nodes:   " << trie_.num_nodes() << endl;
        os << "size:   " << trie_.size_in_bytes() << endl;
    }
    
    void PrintForDebug(std::ostream &os) const {
        
    }
    
};
    
}

#endif /* XcdatWrapper_hpp */

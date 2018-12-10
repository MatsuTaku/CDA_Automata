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
    using Xcdat = xcdat::Trie<FAST>;
    Xcdat xcdat_;
    
public:
    XcdatWrapper(std::istream& is) : xcdat_(is) {}
    
    static std::string name() {
        return typeid(Xcdat).name();
    }
    
    bool Accept(std::string_view str) const {
        return xcdat_.lookup(str) != Xcdat::NOT_FOUND;
    }
    
    size_t Lookup(std::string_view str) const {
        return xcdat_.lookup(str);
    }
    
    std::string Access(size_t value) const {
        return xcdat_.access(value);
    }
    
    void ShowStats(std::ostream& os) const {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "#nodes:   " << xcdat_.num_nodes() << endl;
        os << "size:   " << xcdat_.size_in_bytes() << endl;
    }
    
    void PrintForDebug(std::ostream &os) const {
        
    }
    
};
    
}

#endif /* XcdatWrapper_hpp */

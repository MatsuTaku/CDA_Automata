//
//  XcdatWrapper.hpp
//  ArrayFSA_dev
//
//  Created by 松本拓真 on 2018/07/18.
//

#ifndef XcdatWrapper_hpp
#define XcdatWrapper_hpp

#include "xcdat/Trie.hpp"
#include "csd_automata/util.hpp"

namespace wrapper {
    
template<bool Fast>
class XcdatWrapper {
public:
    using Trie = xcdat::Trie<Fast>;
    
    static std::string name() {
        return typeid(Trie).name();
    }
    
    static std::string tag() {
        return std::string("xcdat") + (Fast ? "-f" : "");
    }
    
    XcdatWrapper() = default;
    
    XcdatWrapper(std::istream& is) : xcdat_(is) {}
    
    friend void LoadFromFile(XcdatWrapper<Fast>& self, std::string file_name);
    
    bool Accept(std::string_view str) const {
        return xcdat_.lookup(str) != Trie::NOT_FOUND;
    }
    
    size_t Lookup(std::string_view str) const {
        return xcdat_.lookup(str);
    }
    
    std::string Access(size_t value) const {
        return xcdat_.access(value);
    }
    
    size_t size_in_bytes() const {
        return xcdat_.size_in_bytes();
    }
    
    void ShowStats(std::ostream& os) const {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "#nodes:   " << xcdat_.num_nodes() << endl;
        os << "size:   " << xcdat_.size_in_bytes() << endl;
    }
    
    void PrintForDebug(std::ostream &os) const {
        
    }
    
private:
    Trie xcdat_;
    
};

void LoadFromFile(XcdatWrapper<true>& self, std::string file_name) {
    auto ifs = csd_automata::util::GetStreamOrDie<std::ifstream>(file_name);
    self.xcdat_ = XcdatWrapper<true>::Trie(ifs);
}

void LoadFromFile(XcdatWrapper<false>& self, std::string file_name) {
    auto ifs = csd_automata::util::GetStreamOrDie<std::ifstream>(file_name);
    self.xcdat_ = XcdatWrapper<false>::Trie(ifs);
}

}

#endif /* XcdatWrapper_hpp */

//
//  CentroidWrapper.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2019/01/04.
//

#ifndef CentroidWrapper_hpp
#define CentroidWrapper_hpp

#include "tries/path_decomposed_trie.hpp"
#include "tries/compressed_string_pool.hpp"

#include "csd_automata/util.hpp"

#include "succinct/mapper.hpp"

namespace wrapper {

class CentroidWrapper {
public:
    using Trie = succinct::tries::path_decomposed_trie<succinct::tries::compressed_string_pool>;
    
    static std::string name() {
        return "centroid-repair";
    }
    
    static std::string tag() {
        return "cent-rp";
    }
    
    CentroidWrapper() = default;
    
    template <typename Visitor>
    void map(Visitor& visit) {
        visit(trie_, "trie_");
    }
    
    bool Accept(const std::string& key) const {
        return trie_.index(key) != -1;
    }
    
    auto Lookup(const std::string& key) const {
        return trie_.index(key);
    }
    
    std::string Access(size_t id) const {
        return trie_[id];
    }
    
    size_t size_in_bytes() {
        return succinct::mapper::size_of(trie_);
    }
    
    void ShowStats(std::ostream &os) {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "num_queries:   " << trie_.size() << endl;
        os << "io_size:   " << size_in_bytes() << endl;
    }
    
    void PrintForDebug(std::ostream& os) const {
        
    }
    
private:
    Trie trie_;
    
};

} // namespace wrapper

#endif /* CentroidWrapper_hpp */

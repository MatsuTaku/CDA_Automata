//
//  DartsCloneWrapper.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/09/29.
//

#ifndef DartsCloneWrapper_hpp
#define DartsCloneWrapper_hpp

#include <darts.h>
#include <fstream>
#include "csd_automata/Exception.hpp"

namespace wrapper {
    
class DartsCloneWrapper {
public:
    static std::string name() {
        return "darts-clone";
    }
    
public:
    DartsCloneWrapper() = default;
    
    // MARK: Build from datasets
    DartsCloneWrapper(const char* fileName) {
        read(fileName);
    }
    
    int build(const char* fileName) {
        std::ifstream ifs(fileName);
        if (!ifs)
            throw csd_automata::exception::DataNotFound(fileName);
        
        // Make key lists from data sets
        std::vector<char *> keys;
        size_t numKeys = 0;
        for (std::string str; std::getline(ifs, str); numKeys++) {
            keys.push_back(new char[str.size() + 1]);
            strcpy(keys.back(), str.c_str());
            keys.back()[str.size()] = '\0';
        }

        auto result = da_.build(numKeys, keys.data());
        
        for (auto &key : keys)
            delete key;
        
        return result;
    }
    
    // MARK: Funcionals
    
    size_t lookup(const char* key) const {
        return da_.exactMatchSearch<Darts::DoubleArray::result_type>(key);
    }
    
    // MARK: IO
    
    void read(const char *fileName) {
        da_.open(fileName);
    }
    
    auto write(const char *fileName) const {
        return da_.save(fileName);
    }
    
    void showStatus(std::ostream &os) const {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "unit_size:   " << da_.unit_size() << endl;
        os << "size:   " << da_.size() << endl;
        os << "total_size:   " << da_.total_size() << endl;
    }
    
private:
    Darts::DoubleArray da_;
    
};
    
}

#endif /* DartsCloneWrapper_hpp */

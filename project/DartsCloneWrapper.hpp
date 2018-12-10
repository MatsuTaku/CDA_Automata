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
#include <vector>

namespace wrapper {
    
class DartsCloneWrapper {
    
    Darts::DoubleArray da_;
    
public:
    
    static std::string name() {
        return "darts-clone";
    }
    
    DartsCloneWrapper() = default;
    
    // MARK: Build from datasets
    DartsCloneWrapper(const char* fileName) {
        Read(fileName);
    }
    
    int Build(std::ifstream& ifs) {
        // Make key lists from data sets
        std::vector<char*> keys;
        size_t num_keys = 0;
        for (std::string str; std::getline(ifs, str); num_keys++) {
            keys.push_back(new char[str.size() + 1]);
            strcpy(keys.back(), str.c_str());
            keys.back()[str.size()] = '\0';
        }

        auto result = da_.build(num_keys, keys.data());
        
        for (auto &key : keys)
            delete key;
        
        return result;
    }
    
    // MARK: Funcionals
    
    size_t Lookup(const char* key) const {
        return da_.exactMatchSearch<Darts::DoubleArray::result_type>(key);
    }
    
    // MARK: IO
    
    void Read(const char *fileName) {
        da_.open(fileName);
    }
    
    auto Write(const char *fileName) const {
        return da_.save(fileName);
    }
    
    void ShowStatus(std::ostream &os) const {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "unit_size:   " << da_.unit_size() << endl;
        os << "size:   " << da_.size() << endl;
        os << "total_size:   " << da_.total_size() << endl;
    }
    
};
    
}

#endif /* DartsCloneWrapper_hpp */

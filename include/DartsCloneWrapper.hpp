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
    
    static std::string tag() {
        return "darts";
    }
    
    DartsCloneWrapper() = default;
    
    friend void LoadFromFile(DartsCloneWrapper& self, const char* file_name);
    
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
    
    void Read(const char* file_name) {
        da_.open(file_name);
    }
    
    auto Write(const char* file_name) const {
        return da_.save(file_name);
    }
    
    size_t size_in_bytes() const {
        return da_.total_size();
    }
    
    void ShowStatus(std::ostream& os) const {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "unit_size:   " << da_.unit_size() << endl;
        os << "size:   " << da_.size() << endl;
        os << "total_size:   " << da_.total_size() << endl;
    }
    
};

void LoadFromFile(DartsCloneWrapper& self, const char* file_name) {
    self.Read(file_name);
}

}

#endif /* DartsCloneWrapper_hpp */

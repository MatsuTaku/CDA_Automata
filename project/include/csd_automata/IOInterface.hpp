//
//  IOInterface.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/10/09.
//

#ifndef IOInterface_hpp
#define IOInterface_hpp

#include "basic.hpp"
#include "util.hpp"
#include <iostream>
#include <fstream>

namespace csd_automata {

class IOInterface {
public:
    // MARK: Protocol methods
    
    virtual size_t size_in_bytes() const = 0;
    virtual void LoadFrom(std::istream&) = 0;
    virtual void StoreTo(std::ostream&) const = 0;
    
    // MARK: Optional methods
    
    virtual void ShowStats(std::ostream& os) const {
        os << "ShowStatus(std::ostream) isn't supported!" << std::endl;
    }
    
};

template <class Product>
inline void LoadFromFile(Product& product, const std::string& file_name) {
    auto ifs = util::GetStreamOrDie<std::ifstream>(file_name);
    product.LoadFrom(ifs);
}

template <class Product>
inline void StoreToFile(Product& product, const std::string& file_name) {
    auto ofs = util::GetStreamOrDie<std::ofstream>(file_name);
    product.StoreTo(ofs);
}

} // namespace csd_automata

#endif /* IOInterface_hpp */

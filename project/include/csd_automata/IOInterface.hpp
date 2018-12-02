//
//  IOInterface.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/10/09.
//

#ifndef IOInterface_hpp
#define IOInterface_hpp

#include <iostream>

namespace csd_automata {
    
class IOInterface {
public:
    // MARK: Protocol methods
    
    virtual size_t size_in_bytes() const = 0;
    virtual void Read(std::istream&) = 0;
    virtual void Write(std::ostream&) const = 0;
    
    // MARK: Optional methods
    
    virtual void ShowStats(std::ostream& os) const {
        os << "ShowStatus(std::ostream) isn't supported!" << std::endl;
    }
    
};
    
} // namespace csd_automata

#endif /* IOInterface_hpp */

//
//  IOInterface.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/10/09.
//

#ifndef IOInterface_hpp
#define IOInterface_hpp

#include "basic.hpp"

namespace csd_automata {
    
    class IOInterface {
    public:
        // MARK: Protocol methods
        
        virtual void read(std::istream&) = 0;
        virtual void write(std::ostream&) const = 0;
        virtual size_t sizeInBytes() const = 0;
        
        // MARK: Optional methods
        
        virtual void showStatus(std::ostream& os) const {
            os << "Not supports showStatus(std::ostream)" << std::endl;
        }
        
    };
    
} // namespace csd_automata

#endif /* IOInterface_hpp */

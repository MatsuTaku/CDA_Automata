//
//  DawgInterface.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/12/15.
//

#ifndef DawgInterface_hpp
#define DawgInterface_hpp

#include "basic.hpp"
#include "CommonPrefixSet.hpp"

namespace csd_automata {
    
class StringDictionaryInterface {
public:
    virtual bool Accept(std::string_view) const = 0;
    virtual id_type Lookup(std::string_view) const = 0;
    virtual std::string Access(id_type) const = 0;
};
    
}

#endif /* DawgInterface_hpp */

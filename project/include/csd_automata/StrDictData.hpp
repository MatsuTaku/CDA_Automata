//
//  StrDictData.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/08.
//

#ifndef StrDictData_hpp
#define StrDictData_hpp

#include "basic.hpp"

namespace csd_automata {
    
    struct StrDictData {
        // Elabled
        bool enabled = true;
        // This data id
        size_t id = 0;
        // place at node
        size_t node_id = 0;
        // this label
        std::string label = "";
        // matched with suffix of other label
        bool isIncluded = false;
        // Data id of included owner
        size_t owner = 0;
        // label placed index at array
        int place = -1;
        // matched counter
        size_t counter = 0;
    
        void set(char c) {
            label += c;
        }
        
        float entropy() const {
            return float(counter) / label.size();
        }
        
    };

}

#endif /* StrDictData_hpp */

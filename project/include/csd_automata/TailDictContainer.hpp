//
//  TailDictContainer.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/12/14.
//

#ifndef TailDictContainer_hpp
#define TailDictContainer_hpp

#include <string_view>

namespace csd_automata {

struct TailDictContainer {
    // Elabled
    bool enabled = true;
    // This data id
    size_t id = 0;
    // place at node
    size_t node_id = 0;
    // this label
    std::string label = "";
    // matched with suffix of other label
    bool is_merged = false;
    // Data id of included owner
    size_t owner = 0;
    // label placed index at array
    int place = -1;
    // matched counter
    size_t counter = 0;
    
    void push_label(char c) {
        label += c;
    }
    
    double entropy() const {
        return double(counter) / label.size();
    }
    
};

}

#endif /* TailDictContainer_hpp */

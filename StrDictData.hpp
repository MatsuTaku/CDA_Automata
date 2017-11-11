//
//  StrDictData.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/08.
//

#ifndef StrDictData_hpp
#define StrDictData_hpp

#include "basic.hpp"

namespace array_fsa {
    
    struct StrDictData {
        std::string label = "";
        size_t place = 0;
        
        void set(char c) {
            label += c;
        }
    };

}

#endif /* StrDictData_hpp */

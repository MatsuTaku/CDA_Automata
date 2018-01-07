//
//  ByteData.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2018/01/05.
//

#ifndef ByteData_hpp
#define ByteData_hpp

#include "basic.hpp"

namespace array_fsa {
    
    class ByteData {
        virtual size_t sizeInBytes() const = 0;
        virtual void read(std::istream &is) = 0;
        virtual void write(std::ostream &os) const = 0;
    };
    
}

#endif /* ByteData_hpp */

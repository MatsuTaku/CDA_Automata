//
//  Calc.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/12/10.
//

#ifndef Calc_hpp
#define Calc_hpp

#include <stdio.h>

namespace array_fsa {
    
    class Calc {
    public:
        static size_t sizeFitInBytes(size_t value) {
            return sizeFitInUnits(value, 8);
        }
        
        static size_t sizeFitInBits(size_t value) {
            return sizeFitInUnits(value, 1);
        }
        
        static size_t sizeFitInUnits(size_t value, size_t unit) {
            if (value == 0)
                return 0;
            auto size = 0;
            while (value >> (unit * ++size))
                if (size > 4)
                    abort();
            return size;
        }
    };
    
}

#endif /* Calc_hpp */

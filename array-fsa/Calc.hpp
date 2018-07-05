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
            while (value >> (unit * ++size));
            return size;
        }
        
        static std::vector<size_t> separateCountsInSizeOf(std::vector<size_t> &list) {
            std::vector<size_t> counts(4);
            for (auto v : list) {
                auto size = sizeFitInBytes(v);
                ++counts[size - 1];
            }
            return counts;
        }
        
        static std::vector<size_t> separateCountsInXorSizeOf(std::vector<size_t> &list) {
            std::vector<size_t> counts(4);
            for (auto i = 0; i < list.size(); i++) {
                auto size = sizeFitInBytes(list[i] ^ i);
                ++counts[size - 1];
            }
            return counts;
        }
    };
    
}

#endif /* Calc_hpp */

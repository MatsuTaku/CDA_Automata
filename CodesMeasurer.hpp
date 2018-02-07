//
//  CodesMeasurer.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/02/06.
//

#ifndef CodesMeasurer_hpp
#define CodesMeasurer_hpp

#include "basic.hpp"
#include "CodeArray.hpp"

namespace array_fsa {
    
    class CodesMeasurer {
    public:
        template <class T>
        static void bench() {
            std::cout << "---- Benchmark of " << T::name() << "----" << std::endl;
            
            const size_t num = 100000;
            std::vector<float_t> secs(4);
            std::vector<size_t> sizes(4);
            
            for (auto byte = 1; byte <= 5; byte++) {
                std::vector<uint8_t> highs(num);
                
                std::vector<size_t> numbers;
                if (byte <= 4) {
                    numbers = byteArray(byte, num);
                } else {
                    numbers = randomArray(num);
                }
                
                CodeArray<T> code;
                code.setCodes(numbers);
                
                Stopwatch sw;
                for (auto i = 0; i < num; i++) {
                    auto n = highs[i] |code.getCode(i) << 8;
                    n = 0;
                }
                auto mSec = sw.get_micro_sec();
                secs[byte - 1] = mSec / num * 1000;
                sizes[byte - 1] = code.sizeInBytes();
                std::cout << byte << " byte time: " << secs[byte - 1] << " ns/" << num << std::endl;
                std::cout << "       size: " << sizes[byte - 1] << std::endl;
            }
        }
        
        static std::vector<size_t> byteArray(size_t size, size_t num) {
            std::vector<size_t> numbers(num);
            for (auto i = 0; i < num; i++) {
                numbers[i] = (1 << (8 * (size - 1))) - 1;
            }
            return numbers;
        }
        
        static std::vector<size_t> randomArray(size_t num) {
            std::vector<size_t> numbers(num);
            for (auto i = 0; i < num; i++) {
                auto size = std::rand() % 4;
                numbers[i] = (1 << (8 * size)) - 1;
            }
            return numbers;
        }
        
    };
    
}

#endif /* CodesMeasurer_hpp */

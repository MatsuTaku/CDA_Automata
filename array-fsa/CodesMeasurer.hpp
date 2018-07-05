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
            
            for (auto byte = 1; byte <= 8; byte++) {
                std::vector<uint8_t> highs(num);
                
                std::vector<size_t> numbers;
                if (byte <= 4) {
                    numbers = byteArray(byte, num);
                } else if (byte == 5) {
                    numbers = randomArray(num);
                } else if (byte == 6) {
                    numbers = curveArray8(num);
                } else if (byte == 7) {
                    numbers = curveArray9(num);
                } else if (byte == 8) {
                    numbers = curveArray44(num);
                }
                
                CodeArray<T> code;
                code.setCodes(numbers);
                
                Stopwatch sw;
                size_t n; // never use
                for (auto i = 0; i < num; i++) {
                    n = highs[i] | (code.getCode(i) << 8);
                }
                auto mSec = sw.get_micro_sec();
                std::cout << byte << " byte time: " << mSec / num * 1000 << " ns/" << num << std::endl;
                std::cout << "       size: " << code.sizeInBytes() << std::endl;
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
        
        static std::vector<size_t> curveArray8(size_t num) {
            std::vector<size_t> numbers(num);
            for (auto i = 0; i < num; i++) {
                auto size = std::rand() % 15;
                if (size > 3) size = 0;
                numbers[i] = (1 << (8 * size)) - 1;
            }
            return numbers;
        }
        
        static std::vector<size_t> curveArray9(size_t num) {
            std::vector<size_t> numbers(num);
            for (auto i = 0; i < num; i++) {
                auto size = std::rand() % 30;
                if (size > 3) size = 0;
                numbers[i] = (1 << (8 * size)) - 1;
            }
            return numbers;
        }
        
        static std::vector<size_t> curveArray44(size_t num) {
            std::vector<size_t> numbers(num);
            for (auto i = 0; i < num; i++) {
                auto size = std::rand() % 10;
                if (size > 3) size = 0;
                if (size > 6) size = 1;
                numbers[i] = (1 << (8 * size)) - 1;
            }
            return numbers;
        }
        
    };
    
}

#endif /* CodesMeasurer_hpp */

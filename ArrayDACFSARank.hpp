//
//  ArrayDACFSARank.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#ifndef ArrayDACFSARank_hpp
#define ArrayDACFSARank_hpp

#include "Rank.hpp"

namespace array_fsa {
    
    class ArrayDACFSARank final : public Rank {
    public:
        bool get_used_dac(size_t index) const {
            return get(index);
        }
        
        bool get_is_final(size_t index) const {
            return (bits_[abs(index)] & (1U << (rel(index) + 1))) != 0;
        }
        
        void set_used_dac(size_t index, bool bit) {
            set(index, bit);
            
            // TODO: Test set needs dac
            //            if (get_used_dac(index) == 0) {
            //                std::cout << "Error set DAC!" << std::endl;
            //            }
        }
        
        void set_is_final(size_t index, bool bit) {
            check_resize(index);
            if (bit) {
                bits_[abs(index)] |= (1U << (rel(index) + 1));
            } else {
                bits_[abs(index)] &= ~(1U << (rel(index) + 1));
            }
            
            // TODO: Test set is final
            //            if (get_is_final(index) == 0) {
            //                std::cout << "Error set isFinal!" << std::endl;
            //            }
        }
        
    private:
        size_t block(size_t index) const override {
            return index / (kBlockSize / 2);
        }
        
        size_t abs(size_t index) const override {
            return index / (kBitSize / 2);
        }
        
        size_t rel(size_t index) const override {
            return index % (kBitSize / 2) * 2;
        }
        
        size_t pop_count(size_t x) const override {
            //            auto source = x;
            //            x = (x & 0x5555555555555555) + ((x >> 1) & 0x5555555555555555);
            //            x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
            x = (x & 0x1111111111111111) + ((x >> 2) & 0x1111111111111111);
            x = (x & 0x0f0f0f0f0f0f0f0f) + ((x >> 4) & 0x0f0f0f0f0f0f0f0f);
            x = (x & 0x00ff00ff00ff00ff) + ((x >> 8) & 0x00ff00ff00ff00ff);
            x = (x & 0x0000ffff0000ffff) + ((x >> 16) & 0x0000ffff0000ffff);
            x = (x & 0x00000000ffffffff) + ((x >> 32) & 0x00000000ffffffff);
            
            // TODO: Test pop count
            //            size_t count1 = 0;
            //            for (auto i = 0; i < 0x20; i++) {
            //                if ((source & 1) == 1) {
            //                    count1++;
            //                }
            //                source >>= 2;
            //            }
            //            if (count1 != x) {
            //                std::cout << "pop count error: " << x << "\t" << count1 << std::endl;
            //            }
            return x;
        }
        
    };
    
}

#endif /* ArrayDACFSARank_hpp */

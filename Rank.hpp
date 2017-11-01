//
//  Rank.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/10/30.
//

#ifndef Rank_hpp
#define Rank_hpp

#include "basic.hpp"

namespace array_fsa {
    
    class Rank {
        typedef uint64_t rankBlock;
        
    public:
        size_t get_used_dac(size_t index) const {
            return (bytes_[abs(index)] >> rel(index)) & 1;
        }
        
        size_t get_is_final(size_t index) const {
            return (bytes_[abs(index)] >> (rel(index) + 1)) & 1;
        }
        
        void set_needs_dac(size_t index) {
            if (abs(index) > bytes_.size() - 1) {
                expand(abs(index));
            }
            bytes_[abs(index)] |= (rankBlock(1) << rel(index));
            
            // TODO: Test set needs dac
//            if (get_used_dac(index) == 0) {
//                std::cout << "Error set DAC!" << std::endl;
//            }
        }
        
        void set_is_final(size_t index) {
            if (abs(index) + 1 > bytes_.size()) {
                expand(abs(index));
            }
            bytes_[abs(index)] |= (rankBlock(1) << (rel(index) + 1));
            
            // TODO: Test set is final
//            if (get_is_final(index) == 0) {
//                std::cout << "Error set isFinal!" << std::endl;
//            }
        }
        
        void expand(size_t size) {
            bytes_.resize(size + 1);
            auto beforeCSize = count_bytes_.size();
            count_bytes_.resize(size);
            if (size > 1) {
                for (auto i = beforeCSize; i < count_bytes_.size(); i++) {
                    count_bytes_[i] = count_bytes_[i - 1] + pop_count(bytes_[i]);
                }
            } else if (size == 1) {
                count_bytes_[size - 1] = pop_count(bytes_[size - 1]);
            }
        }
        
        size_t rank(size_t index) const {
            size_t offset = 0;
            auto abs_index = abs(index);
            if (abs_index > 0) {
                offset += count_bytes_[abs_index - 1];
            }
            auto rel_index = rel(index);
            rankBlock mask = -1;
            if (rel_index < 0x3f) {
                mask = (rankBlock(1) << (rel_index + 1)) - 1;
            }
            offset += pop_count(bytes_[abs_index] & mask);
            
            return offset;
        }
        
        void write(std::ostream& os) const {
            write_vec(bytes_, os);
            write_vec(count_bytes_, os);
        }
        void read(std::istream& is) {
            bytes_ = read_vec<rankBlock>(is);
            count_bytes_ = read_vec<size_t>(is);
        }
        
        size_t size_in_bytes() const {
            return size_vec(bytes_) + size_vec(count_bytes_);
        }
        
        void swap(Rank &rank) {
            bytes_.swap(rank.bytes_);
            count_bytes_.swap(rank.count_bytes_);
        }
        
        static void show_as_bytes(rankBlock value, size_t size) {
            for (int i = size * 8 - 1; i >= 0; i--) {
                std::cout << ((value >> i & 1) == 1);
            }
            std::cout << std::endl;
        }
        
    private:
        std::vector<rankBlock> bytes_;
        std::vector<size_t> count_bytes_;
        
        size_t abs(size_t index) const {
            return (index * 2) / 0x40;
        }
        
        size_t rel(size_t index) const {
            return (index * 2) % 0x40;
        }
        
        size_t pop_count(size_t x) const {
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

#endif /* Rank_hpp */

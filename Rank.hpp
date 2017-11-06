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
    protected:
        using rankBlock = uint32_t;
        static constexpr size_t kBlockSize { 0x100 };
        static constexpr uint8_t kBitSize { 32 };
        static constexpr uint8_t kBlockInTipSize { kBlockSize / kBitSize }; // 8
        
    public:
        bool get(size_t index) const {
            return (bits_[abs(index)] & (1U << rel(index))) != 0;
        }
        
        void set(size_t index, bool bit) {
            check_resize(index);
            if (bit) {
                bits_[abs(index)] |= (1U << rel(index));
            } else {
                bits_[abs(index)] &= ~(1U << rel(index));
            }
        }
        
        void build() {
            rank_tips_.resize(bits_.size() / kBlockInTipSize + 1);
            size_t count = 0;
            for (auto i = 0; i < rank_tips_.size(); i++) {
                auto& tip = rank_tips_[i];
                tip.L1 = count;
                for (auto offset = 0; offset < kBlockInTipSize; offset++) {
                    tip.L2[offset] = count - tip.L1;
                    auto index = i * kBlockInTipSize + offset;
                    if (index < bits_.size()) {
                        count += pop_count(bits_[index]);
                    }
                }
            }
        }
        
        void check_resize(size_t index) {
            if (abs(index) + 1 > bits_.size()) {
                resize(abs(index) + 1);
            }
        }
        
        void resize(size_t size) {
            bits_.resize(size);
        }
        
        size_t rank(size_t index) const {
            auto& tip = rank_tips_[block(index)];
            return tip.L1 + tip.L2[abs(index) % kBlockInTipSize] + pop_count(bits_[abs(index)] & (-1U >> (kBitSize - rel(index + 1))));
        }
        
        void write(std::ostream& os) const {
            write_vec(bits_, os);
            write_vec(rank_tips_, os);
        }
        void read(std::istream& is) {
            bits_ = read_vec<rankBlock>(is);
            rank_tips_ = read_vec<RankTip>(is);
        }
        
        size_t size_in_bytes() const {
            return size_vec(bits_) + size_vec(rank_tips_);
        }
        
        void swap(Rank &rank) {
            bits_.swap(rank.bits_);
            rank_tips_.swap(rank.rank_tips_);
        }
        
        static void show_as_bytes(rankBlock value, size_t size) {
            for (int i = size * 8 - 1; i >= 0; i--) {
                std::cout << ((value >> i & 1) == 1);
            }
            std::cout << std::endl;
        }
        
    protected:
        std::vector<rankBlock> bits_;
        struct RankTip {
            rankBlock L1;
            uint8_t L2[kBlockInTipSize];
        };
        std::vector<RankTip> rank_tips_;
        
        virtual size_t block(size_t index) const {
            return index / kBlockSize;
        }
        
        virtual size_t abs(size_t index) const {
            return index / kBitSize;
        }
        
        virtual size_t rel(size_t index) const {
            return index % kBitSize;
        }
        
        virtual size_t pop_count(size_t x) const {
            x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
            x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
            x = (x & 0x0f0f0f0f) + ((x >> 4) & 0x0f0f0f0f);
            x += (x >> 8);
            x += (x >> 16);
            return x & 0x3F;
            return x;
        }
        
    };
    
}

#endif /* Rank_hpp */

//
//  MultiBitVector.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/03/30.
//

#ifndef MultiBitVector_hpp
#define MultiBitVector_hpp

#include "basic.hpp"
#include "Log.hpp"

namespace array_fsa {
    
    class MultiBitVector : ByteData {
    public:
        using idType = uint64_t;
        static constexpr uint8_t kBitsUnitSize = 2;
        static constexpr uint8_t kByteSize = 1 << kBitsUnitSize;
        static constexpr idType kBitsMask = kByteSize - 1;
        static constexpr size_t kBlockSize = 0x200;
        static constexpr uint8_t kBitSize = 0x40;
        static constexpr uint8_t kBlockInTipSize = kBlockSize / kBitSize; // 8
        static constexpr uint8_t kBits[1 << kBitsUnitSize] = {0b00, 0b01, 0b10, 0b11};
        
        MultiBitVector() = default;
        ~MultiBitVector() = default;
        
        // MARK: - getter
        
        uint8_t operator[](size_t index) const {
            return bits_[abs(index)] >> rel(index) & kBitsMask;
        }
        
        size_t rank(size_t index) const {
            const auto &tip = rank_tips_[block(index)];
            auto bits = bits_[abs(index)] >> rel(index) & kBitsMask;
            assert(bits > 0);
            auto b = bits - 1;
            return tip.L1[b] + tip.L2[abs(index) % kBlockInTipSize][b] + callPopCount(bits, bits_[abs(index)] & ((1LU << rel(index)) - 1));
        }
        
        void build() {
            if (bits_.size() == 0) return;
            
            rank_tips_.resize(bits_.size() / kBlockInTipSize + 1);
            std::vector<idType> count(kByteSize - 1, 0);
            for (auto i = 0; i < rank_tips_.size(); i++) {
                auto &tip = rank_tips_[i];
                // If bits = 0b00, don't make rank dict!
                for (auto bits = 1; bits < kByteSize; bits++) {
                    auto b = kBits[bits] - 1;
                    tip.L1[b] = count[b];
                }
                for (auto offset = 0; offset < kBlockInTipSize; offset++) {
                    // Same reason!
                    for (auto bits = 1; bits < kByteSize; bits++) {
                        auto b = bits - 1;
                        tip.L2[offset][b] = count[b] - tip.L1[b];
                        auto index = i * kBlockInTipSize + offset;
                        if (index < bits_.size()) {
                            count[b] += callPopCount(bits, bits_[index]);
                        }
                    }
                }
            }
        }
        
        // MARK: - setter
        
        void set(size_t index, uint8_t value) {
            assert(value <= kBitsMask);
            checkResize(index);
            auto ri = rel(index);
            auto &obj = bits_[abs(index)];
            obj = (obj & ~(kBitsMask << ri)) | (idType(value) << ri);
        }
        
        void checkResize(size_t index) {
            if (abs(index) < bits_.size()) return;
            resize(index);
        }
        
        void resize(size_t index) {
            bits_.resize(abs(index) + 1);
        }
        
        // MARK: - ByteData
        
        size_t sizeInBytes() const override {
            auto size = size_vec(bits_);
            size += size_vec(rank_tips_);
            return size;
        }
        
        void write(std::ostream &os) const override {
            write_vec(bits_, os);
            write_vec(rank_tips_, os);
        }
        
        void read(std::istream &is) override {
            bits_ = read_vec<idType>(is);
            rank_tips_ = read_vec<RankTip>(is);
        }
        
        // MARK: - Copy guard
        
        MultiBitVector(const MultiBitVector&) = delete;
        MultiBitVector& operator=(const MultiBitVector&) = delete;
        
        MultiBitVector(MultiBitVector&&) noexcept = default;
        MultiBitVector& operator=(MultiBitVector&&) noexcept = default;
        
    private:
        std::vector<idType> bits_;
        struct RankTip {
            idType L1[kByteSize - 1];
            uint8_t L2[kBlockInTipSize][kByteSize - 1];
        };
        std::vector<RankTip> rank_tips_;
        
        // MARK: - private methods
        
        size_t block(size_t index) const {
            return index * kBitsUnitSize / kBlockSize;
        }
        
        size_t abs(size_t index) const {
            return index * kBitsUnitSize / kBitSize;
        }
        
        uint8_t rel(size_t index) const {
            return index * kBitsUnitSize % kBitSize;
        }
        
        template <uint8_t T>
        uint8_t popCount(idType x) const {
            assert(T < 0b100);
            if (T == 0b00)
                x = ((~x >> 1) & 0x5555555555555555) + (~x & 0x5555555555555555);
            else if (T == 0b01)
                x = ((~x >> 1) & 0x5555555555555555) + (x & 0x5555555555555555);
            else if (T == 0b10)
                x = ((x >> 1) & 0x5555555555555555) + (~x & 0x5555555555555555);
            else if (T == 0b11)
                x = ((x >> 1) & 0x5555555555555555) + (x & 0x5555555555555555);
            x = ((x >> 1) & 0x1111111111111111) + ((x >> 3) & 0x1111111111111111);
            x = (x & 0x0f0f0f0f0f0f0f0f) + ((x >> 4) & 0x0f0f0f0f0f0f0f0f);
            x += x >> 8;
            x += x >> 16;
            x += x >> 32;
            return x & 0xFF; // 0x7f
        }
        
        uint8_t callPopCount(uint8_t bits, idType value) const {
            assert(bits < 0b100);
            switch (bits) {
                case 0b00:
                    return popCount<0b00>(value);
                case 0b01:
                    return popCount<0b01>(value);
                case 0b10:
                    return popCount<0b10>(value);
                case 0b11:
                    return popCount<0b11>(value);
                default:
                    abort();
                    return 0;
            }
        }
        
    };
    
}

#endif /* MultiBitVector_hpp */

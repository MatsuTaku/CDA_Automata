//
//  DACs.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2018/01/07.
//

#ifndef DACs_hpp
#define DACs_hpp

#include "ByteData.hpp"

#include "BitVector.hpp"
#include "Calc.hpp"

namespace array_fsa {
    
    template<bool LINK = false>
    class DACs : ByteData {
    public:
        static constexpr bool useLink = LINK;
    public:
        // MARK: Constructor
        
        DACs() = default;
        ~DACs() = default;
        
        DACs(const DACs&) = delete;
        DACs& operator =(const DACs&) = delete;
        
        DACs(DACs&& rhs) noexcept = default;
        DACs& operator =(DACs&& rhs) noexcept = default;
        
        // MARK: Function
        
        static std::string name() {
            return "DACs";
        }
        
        void setUnitSize(size_t size) {
            unit_size_ = size;
        }
        
        void expand(size_t size) {
            if (num_units_ >= size) return;
            num_units_ = size;
            bits_list_.resize(size);
            units_.resize(size);
        }
        
        bool getBitInFirstUnit(size_t index) const {
            return bits_list_[0][index];
        }
        
        void setBitInFirstUnit(size_t index, bool bit) {
            if (num_units_ == 0) expand(1);
            return bits_list_[0].set(index, bit);
        }
        
        void build() {
            for (auto &bits : bits_list_)
                bits.build(true, false);
        }
        
        size_t operator [](size_t index) const {
            return getValue(index);
        }
        
        size_t getValue(size_t index) const;
        
        void setValue(size_t index, size_t value);
        
        void showStats(std::ostream &os) const {
            using std::endl;
            os << "--- Stat of " << "DACs " << " ---" << endl;
            os << "size:   " << sizeInBytes() << endl;
        }
        
        size_t sizeInBytes() const override {
            auto size = sizeof(unit_size_) + sizeof(num_units_);
            for (auto &bits: bits_list_)
                size += bits.sizeInBytes();
            for (auto &unit: units_)
                size += size_vec(unit);
            return size;
        }
        
        void write(std::ostream &os) const override {
            write_val(unit_size_, os);
            write_val(num_units_, os);
            for (auto &bits: bits_list_)
                bits.write(os);
            for (auto &unit: units_)
                write_vec(unit, os);
        }
        
        void read(std::istream &is) override {
            unit_size_ = read_val<size_t>(is);
            num_units_ = read_val<size_t>(is);
            bits_list_.resize(num_units_);
            units_.resize(num_units_);
            for (auto &bits: bits_list_)
                bits.read(is);
            for (auto i = 0; i < num_units_; i++)
                units_[i] = read_vec<uint8_t>(is);
        }
        
    private:
        size_t unit_size_ = 1;
        size_t num_units_ = 0;
        std::vector<BitVector> bits_list_;
        std::vector<std::vector<uint8_t>> units_;
        bool use_link_ = false;
        
    };
    
    // MARK: - inline function
    
    template<bool L>
    inline size_t DACs<L>::getValue(size_t index) const {
        size_t value = 0;
        auto depth = 0;
//        std::cout << "---" << std::endl << index << std::endl;
//        Log::showAsBytes(index, 8);
        for (auto &bits : bits_list_) {
            if (!bits[index]) break;
            index = bits.rank(index);
//            std::cout << index << std::endl;
//            Log::showAsBytes(index, 8);
            auto &unit = units_[depth];
            const auto offset = index * unit_size_;
            const auto shiftSize = depth * unit_size_;
            for (auto i = 0; i < unit_size_; i++)
                value |= unit[offset + i] << ((shiftSize + i) * 8);
            depth++;
        }
        return value;
    }
    
    template<bool L>
    inline void DACs<L>::setValue(size_t index, size_t value) {
        auto size = Calc::sizeFitInUnits(value, unit_size_ * 8);
        if (size > num_units_)
            expand(size);
        if (L) {
            bits_list_[0].set(index, true);
            for (auto i = 0; i < unit_size_; i++)
                units_[0].push_back(value >> (i * 8) & 0xff);
        } else {
            for (auto depth = 0; depth < size; depth++) {
                bits_list_[depth].set(index, true);
                auto &unit = units_[depth];
                const auto shiftSize = depth * unit_size_;
                for (auto i = 0; i < unit_size_; i++)
                    unit.push_back((value >> ((shiftSize + i) * 8)) & 0xff);
                index = unit.size() / unit_size_ - 1;
            }
        }
    }
    
}

#endif /* DACs_hpp */

//
//  DACs.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2018/01/07.
//

#ifndef DACs_hpp
#define DACs_hpp

#include "ByteData.hpp"

#include "DacUnit.hpp"
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
        
        void setUnitSize(uint8_t size) {
            unit_size_ = size;
        }
        
        void expand(size_t size) {
            if (num_units_ >= size) return;
            units_.resize(size);
            for (auto i = num_units_; i < size; i++) {
                units_[i].setUnitSize(unit_size_);
            }
            num_units_ = size;
        }
        
        bool getBitInFirstUnit(size_t index) const {
            return units_[0].getBit(index);
        }
        
        void setBitInFirstUnit(size_t index, bool bit) {
            if (num_units_ == 0) expand(1);
            units_[0].setBit(index, bit);
        }
        
        void build() {
            for (auto &unit : units_)
                unit.build();
        }
        
        size_t operator [](size_t index) const {
            return getValue(index);
        }
        
        size_t getValue(size_t index) const;
        
        void setValue(size_t index, size_t value);
        
        size_t sizeInBytes() const override {
            auto size = sizeof(unit_size_) + sizeof(num_units_);
            for (const auto &unit : units_) {
                size += unit.sizeInBytes();
            }
            return size;
        }
        
        void write(std::ostream &os) const override {
            write_val(unit_size_, os);
            write_val(num_units_, os);
            for (const auto &unit : units_) {
                unit.write(os);
            }
        }
        
        void read(std::istream &is) override {
            unit_size_ = read_val<uint8_t>(is);
            num_units_ = read_val<size_t>(is);
            units_.resize(num_units_);
            for (auto i = 0; i < num_units_; i++) {
                DacUnit unit(is);
                units_[i] = std::move(unit);
            }
        }
        
    private:
        uint8_t unit_size_ = 1;
        size_t num_units_ = 0;
        std::vector<DacUnit> units_ = {};
        bool use_link_ = false;
        
    };
    
    // MARK: - inline function
    
    template<bool L>
    inline size_t DACs<L>::getValue(size_t index) const {
        size_t value = 0;
        auto depth = 0;
        for (const auto &unit : units_) {
            if (!unit.getBit(index)) break;
            index = unit.rank(index);
            value |= (unit.getByteUnit(index) << (depth * 8 * unit_size_));
            depth++;
        }
        return value;
    }
    
    template<bool L>
    inline void DACs<L>::setValue(size_t index, size_t value) {
        auto size = Calc::sizeFitInUnits(value, unit_size_ * 8);
        if (size > num_units_)
            expand(size);
        auto mask = (size_t(1) << (8 * unit_size_)) - 1;
        auto depth = 0;
        for (auto &unit : units_) {
            if (value == 0 && (!L || depth > 0)) {
                unit.setBit(index, false);
                break;
            }
            unit.setBit(index, true);
            unit.setByte(value & mask);
            index = unit.size() - 1;
            value >>= (8 * unit_size_);
            
            depth++;
        }
    }
    
}

#endif /* DACs_hpp */

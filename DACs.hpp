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
    
    class DACs : ByteData {
        friend class NArrayFSATextEdge;
    public:
        DACs() = default;
        ~DACs() = default;
        
        DACs(DACs &&rhs) noexcept : DACs() {
            this->swap(rhs);
        }
        DACs &operator=(DACs &&rhs) noexcept {
            this->swap(rhs);
            return *this;
        }
        
        DACs(const DACs&) = delete;
        DACs &operator=(const DACs&) = delete;
        
        void setUnitSize(uint8_t size) {
            unit_size_ = size;
        }
        
        void setMaxValue(size_t max) {
            auto size = Calc::sizeFitInUnits(max, unit_size_ * 8);
            if (num_units_ >= size) return;
            num_units_ = size;
            for (auto i = units_.size(); i < size; i++) {
                DacUnit unit;
                unit.setUnitSize(unit_size_);
                units_.push_back(unit);
            }
        }
        
        void build() {
            for (auto &unit : units_) {
                unit.build();
            }
        }
        
        size_t getValue(size_t index) const {
            size_t value = 0;
            auto depth = 0;
            for (auto &unit : units_) {
                if (!unit.getBit(index)) break;
                value |= unit.getByte(index) << (8 * unit_size_ * depth);
                index = unit.rank(index) - 1;
                depth++;
            }
            return value;
        }
        
        size_t getMask() const {
            return (size_t(1) << (8 * unit_size_)) - 1;
        }
        
        void useLink(bool use) {
            use_link_ = use;
        }
        
        void setValue(size_t index, size_t value) {
            auto mask = getMask();
            auto depth = 0;
            for (auto &unit : units_) {
                if (value == 0 && (!use_link_ || depth > 0)) break;
                unit.setBit(index, true);
                unit.setByte(value & mask);
                index = unit.size() - 1;
                value >>= (8 * unit_size_);
                
                depth++;
            }
        }
        
        size_t sizeInBytes() const override {
            auto size = sizeof(unit_size_) + sizeof(num_units_);
            for (auto &unit : units_) {
                size += unit.size_in_bytes();
            }
            return size;
        }
        
        void write(std::ostream &os) const override {
            write_val(unit_size_, os);
            write_val(num_units_, os);
            for (auto &unit : units_) {
                unit.write(os);
            }
        }
        
        void read(std::istream &is) override {
            unit_size_ = read_val<uint8_t>(is);
            num_units_ = read_val<size_t>(is);
            for (auto i = 0; i < num_units_; i++) {
                DacUnit unit;
                unit.read(is);
                units_.push_back(unit);
            }
        }
        
        void swap(DACs &rhs) {
            std::swap(unit_size_, rhs.unit_size_);
            std::swap(num_units_, rhs.num_units_);
            for (auto unit : rhs.units_) {
                units_.push_back(unit);
            }
        }
        
    private:
        uint8_t unit_size_ = 1;
        size_t num_units_ = 0;
        std::vector<DacUnit> units_ = {};
        bool use_link_ = false;
    };
    
}

#endif /* DACs_hpp */

//
//  SACs.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/20.
//

#ifndef SACs_hpp
#define SACs_hpp

#include "ByteData.hpp"

#include "DacUnit.hpp"
#include "Calc.hpp"

namespace array_fsa {
    
    template<bool LINK = false>
    class SACs : ByteData {
    public:
        static constexpr bool useLink = LINK;
    public:
        // MARK: Constructor
        
        SACs() = default;
        ~SACs() = default;
        
        SACs(const SACs&) = delete;
        SACs& operator =(const SACs&) = delete;
        
        SACs(SACs&& rhs) noexcept = default;
        SACs& operator =(SACs&& rhs) noexcept = default;
        
        // MARK: Function
        
        static std::string name() {
            return "SACs";
        }
        
        void setUnitSize(uint8_t size) {
            unit_size_ = size;
        }
        
        void expand(size_t size) {
            if (num_units_ >= size) return;
            units_.resize(size);
            for (auto i = num_units_; i < size; i++) {
                DacUnit unit;
                unit.setUnitSize(unit_size_ * (i + 1));
                units_[i] = std::move(unit);
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
            size += first_bits_.sizeInBytes();
            return size;
        }
        
        void write(std::ostream &os) const override {
            write_val(unit_size_, os);
            first_bits_.write(os);
            write_val(num_units_, os);
            for (const auto &unit : units_) {
                unit.write(os);
            }
        }
        
        void read(std::istream &is) override {
            unit_size_ = read_val<uint8_t>(is);
            first_bits_.read(is);
            num_units_ = read_val<size_t>(is);
            units_.resize(num_units_);
            for (auto i = 0; i < num_units_; i++) {
                DacUnit unit(is);
                units_[i] = std::move(unit);
            }
        }
        
    private:
        uint8_t unit_size_ = 1;
        BitVector first_bits_;
        size_t num_units_ = 0;
        std::vector<DacUnit> units_ = {};
        
    };
    
    // MARK: - inline function
    
    template <bool L>
    inline size_t SACs<L>::getValue(size_t index) const {
        if (first_bits_[index]) return 0;
        for (const auto &unit : units_) {
            if (unit.getBit(index))
                return unit.getByteUnit(unit.rank(index));
        }
        return 0;
    }
    
    template <bool L>
    inline void SACs<L>::setValue(size_t index, size_t value) {
        // If using link, units size equal 1.
        assert(!(L && units_.size() > 1));
        if (value == 0 && !L) {
            first_bits_.set(index, true);
            return;
        }
        first_bits_.set(index, false);
        auto size = Calc::sizeFitInUnits(value, unit_size_ * 8);
        if (size > num_units_)
            expand(size);
        for (auto level = 0; level < units_.size(); level++) {
            auto &unit = units_[level];
            if (level == size - 1) {
                unit.setBit(index, true);
                unit.setByte(value);
            } else {
                unit.setBit(index, false);
            }
        }
    }
    
}

#endif /* SACs_hpp */

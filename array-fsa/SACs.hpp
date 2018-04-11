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
#include "MultiBitVector.hpp"
#include "Log.hpp"

namespace array_fsa {
    
    class SACs : ByteData {
    public:
        static constexpr bool useLink = false;
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
            num_units_ = size;
        }
        
        bool getBitInFirstUnit(size_t index) const {
            return multi_bits_[index] == 0b00;
        }
        
        void setBitInFirstUnit(size_t index, bool bit) {
            if (num_units_ == 0) expand(1);
            if (bit)
                multi_bits_.set(index, 0b00);
        }
        
        void build() {
            multi_bits_.build();
        }
        
        size_t operator [](size_t index) const {
            return getValue(index);
        }
        
        size_t getValue(size_t index) const;
        
        void setValue(size_t index, size_t value);
        
        void showStats(std::ostream &os) const {
            using std::endl;
            os << "--- Stat of " << "SACs " << " ---" << endl;
            os << "size:   " << sizeInBytes() << endl;
            os << "size multi_bits:   " << multi_bits_.sizeInBytes() << endl;
            auto sizeFlow = 0;
            for (auto u : units_)
                sizeFlow += size_vec(u);
            os << "size flows:   " << sizeFlow << endl;
        }
        
        size_t sizeInBytes() const override {
            auto size = sizeof(unit_size_) + sizeof(num_units_);
            for (const auto &unit : units_) {
                size += size_vec(unit);
            }
            size += multi_bits_.sizeInBytes();
            return size;
        }
        
        void write(std::ostream &os) const override {
            write_val(unit_size_, os);
            multi_bits_.write(os);
            write_val(num_units_, os);
            for (const auto &unit : units_) {
                write_vec(unit, os);
            }
        }
        
        void read(std::istream &is) override {
            unit_size_ = read_val<uint8_t>(is);
            multi_bits_.read(is);
            num_units_ = read_val<size_t>(is);
            units_.resize(num_units_);
            for (auto i = 0; i < num_units_; i++) {
                units_[i] = read_vec<uint8_t>(is);
            }
        }
        
    private:
        uint8_t unit_size_ = 1;
        MultiBitVector multi_bits_;
        size_t num_units_ = 0;
        std::vector<std::vector<uint8_t>> units_;
        
    };
    
    // MARK: - inline function
    
    inline size_t SACs::getValue(size_t index) const {
        auto size = multi_bits_[index];
        if (size == 0) return 0;
        size_t value = 0;
        auto &unit = units_[size - 1];
        auto offset = multi_bits_.rank(index) * size;
        for (auto i = 0, max = size * unit_size_; i < max; i++)
            value |= unit[offset + i] << (i * 8);
        return value;
    }
    
    inline void SACs::setValue(size_t index, size_t value) {
        if (value == 0) {
            multi_bits_.set(index, 0);
            return;
        }
        auto size = Calc::sizeFitInUnits(value, unit_size_ * 8);
        multi_bits_.set(index, size);
        if (size > num_units_)
            expand(size);
        
        auto &unit = units_[size - 1];
        for (auto i = 0; i < size * unit_size_; i++)
            unit.push_back((value >> (i * 8)) & 0xff);
    }
    
}

#endif /* SACs_hpp */

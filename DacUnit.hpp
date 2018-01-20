//
//  DacUnit.hpp
//
//  Created by 松本拓真 on 2017/11/13.
//

#ifndef DacUnit_hpp
#define DacUnit_hpp

#include "basic.hpp"
#include "Rank.hpp"

namespace array_fsa {
    
    class DacUnit : ByteData {
    public:
        DacUnit() = default;
        DacUnit(std::istream& is) {
            read(is);
        }
        
        ~DacUnit() = default;
        
        size_t size() const {
            return unit_size_ > 0 ? bytes_.size() / unit_size_ : 0;
        }
        
        bool getBit(size_t index) const {
            return bits_.get(index);
        }
        
        size_t getByteUnit(size_t rank) const {
            size_t byte = 0;
            for (auto i = 0; i < unit_size_; i++)
                byte |= bytes_[rank * unit_size_ + i] << (i * 8);
            return byte;
        }
        
        size_t rank(size_t index) const {
            return bits_.rank(index);
        }
        
        void setBit(size_t index, bool bit) {
            bits_.set(index, bit);
        }
        
        void setByte(size_t value) {
            assert(value >> (8 * unit_size_) == 0);
            
            for (auto size = 0; size < unit_size_; size++)
                bytes_.push_back((value >> (size * 8)) & 0xff);
        }
        
        void setUnitSize(uint8_t size) {
            unit_size_ = size;
        }
        
        void build() {
            bits_.build(true, false);
        }
        
        size_t sizeInBytes() const override {
            auto size = bits_.sizeInBytes();
            size += size_vec(bytes_);
            return size;
        }
        
        void read(std::istream& is) override {
            bits_.read(is);
            bytes_ = read_vec<uint8_t>(is);
            unit_size_ = read_val<uint8_t>(is);
        }
        
        void write(std::ostream& os) const override {
            bits_.write(os);
            write_vec(bytes_, os);
            write_val(unit_size_, os);
        }
        
        DacUnit(const DacUnit&) = delete;
        DacUnit& operator =(const DacUnit&) = delete;
        
        DacUnit(DacUnit&&) noexcept = default;
        DacUnit& operator =(DacUnit&&) noexcept = default;
        
    private:
        Rank bits_;
        std::vector<uint8_t> bytes_;
        
        uint8_t unit_size_ = 1;
        
    };
    
}

#endif /* DacUnit_hpp */

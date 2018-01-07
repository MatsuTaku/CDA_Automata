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
    
    struct DacUnit {
    public:
        size_t size() const {
            return bytes_.size() / unit_size_;
        }
        
        bool getBit(size_t index) const {
            return bits_.get(index);
        }
        
        size_t getByte(size_t index) const {
            size_t byte = 0;
            std::memcpy(&byte, &bytes_[(rank(index) - 1) * unit_size_], unit_size_);
            return byte;
        }
        
        size_t rank(size_t index) const {
            return bits_.rank(index);
        }
        
        void setBit(size_t index, bool bit) {
            bits_.set(index, bit);
        }
        
        void setByte(size_t value) {
            if (unit_size_ == 1) {
                bytes_.push_back(value);
            } else {
                if (value >> (8 * unit_size_) != 0) {
                    std::cout << "Set Dac element over flow!" << std::endl;
                    return;
                }
                for (auto size = 0; size < unit_size_; size++) {
                    uint8_t byte = (value >> (size * 8)) & 0xff;
                    bytes_.push_back(byte);
                }
            }
        }
        
        void setUnitSize(uint8_t size) {
            unit_size_ = size;
        }
        
        void build() {
            bits_.build(true, false);
        }
        
        size_t size_in_bytes() const {
            return bits_.size_in_bytes() + size_vec(bytes_);
        }
        
        void read(std::istream &is) {
            bits_.read(is);
            bytes_ = read_vec<uint8_t>(is);
            unit_size_ = read_val<uint8_t>(is);
        }
        
        void write(std::ostream &os) const {
            bits_.write(os);
            write_vec(bytes_, os);
            write_val(unit_size_, os);
        }
        
    private:
        Rank bits_;
        std::vector<uint8_t> bytes_;
        
        uint8_t unit_size_ = 1;
        
    };
    
}

#endif /* DacUnit_hpp */

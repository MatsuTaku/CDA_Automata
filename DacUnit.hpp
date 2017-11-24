//
//  DacUnit.hpp
//
//  Created by 松本拓真 on 2017/11/13.
//

#ifndef DacUnit_hpp
#define DacUnit_hpp

#include "basic.hpp"

namespace array_fsa {
    
    struct DacUnit {
    public:
        size_t size() const {
            return bytes_.size();
        }
        
        bool get(size_t index) const {
            return bits_.get(index);
        }
        
        uint8_t getByte(size_t index) const {
            return bytes_[(rank(index) - 1) * byte_size_];
        }
        
        size_t numBytes() const {
            return bytes_.size();
        }
        
        size_t rank(size_t index) const {
            return bits_.rank(index);
        }
        
        void setBit(size_t index, bool bit) {
            bits_.set(index, bit);
        }
        
        void setOverByte(size_t value) {
            if (value >> (8 * byte_size_) != 0) {
                std::cout << "Set Dac Unit Over flow!" << std::endl;
                return;
            }
            for (auto size = 0; value >> (8 * size); size++) {
                uint8_t byte = (value >> size) & 0xff;
                setByte(byte);
            }
        }
        
        void setByte(uint8_t value) {
            bytes_.push_back(value);
        }
        
        void setByteSize(uint8_t size) {
            byte_size_ = size;
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
        }
        
        void write(std::ostream &os) const {
            bits_.write(os);
            write_vec(bytes_, os);
        }
        
    private:
        Rank bits_;
        uint8_t byte_size_ = 1;
        std::vector<uint8_t> bytes_;
        
    };
    
}

#endif /* DacUnit_hpp */

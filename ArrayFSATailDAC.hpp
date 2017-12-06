//
//  ArrayFSATailDAC.hpp
//  bench
//
//  Created by 松本拓真 on 2017/11/14.
//

#ifndef ArrayFSATailDAC_hpp
#define ArrayFSATailDAC_hpp

#include "ArrayFSATail.hpp"
#include "DacUnit.hpp"

namespace array_fsa {
    
    class ArrayFSATailDACBuilder;
    
    class ArrayFSATailDAC : public ArrayFSATail {
        friend class ArrayFSATailDACBuilder;
        
    public:
        ArrayFSATailDAC() = default;
        virtual ~ArrayFSATailDAC() = default;
        
        ArrayFSATailDAC(ArrayFSATailDAC &&rhs) noexcept : ArrayFSATailDAC() {
            this->swap(rhs);
        }
        ArrayFSATailDAC &operator=(ArrayFSATailDAC &rhs) noexcept {
            this->swap(rhs);
            return *this;
        }
        
        using Builder = ArrayFSATailDACBuilder;
        
        static std::string name() {
            return "ArrayFSATailDAC";
        }
        
        size_t get_next_(size_t trans) const override {
            size_t next = 0;
            std::memcpy(&next, &bytes_[offset_(trans)], 1);
            // TODO: DAC
            for (size_t size = 1, index = trans; size < next_size_; size++) {
                auto &unit = dac_next_units_[size - 1];
                if (!unit.getBit(index)) {
                    break;
                }
                next |= unit.getByte(index) << (8 * size);
                index = unit.rank(index) - 1;
            }
            return next;
        }
        
        void set_next(size_t trans, size_t next) override {
            size_t mask = 0xff;
            auto firstUnit = next & mask;
            std::memcpy(&bytes_[offset_(trans)], &firstUnit, 1);
            auto nextSize = 0;
            while (next >> (8 * ++nextSize));
            for (size_t size = 1, index = trans; size < nextSize; size++) {
                auto &unit = dac_next_units_[size - 1];
                unit.setBit(index, true);
                auto byte = (next >> (8 * size)) & mask;
                unit.setByte(byte);
                index = unit.size() - 1;
            }
        }
        
        void set_check(size_t trans, uint8_t check) override {
            bytes_[offset_(trans) + 1] = check;
        }
        
        void calc_next_size(size_t num_elems) override {
            next_size_ = 0;
            while ((num_elems - 1) >> (8 * ++next_size_));
            dac_next_units_.resize(next_size_ - 1);
        }
        
        void buildBits() override {
            ArrayFSATail::buildBits();
            for (auto &u : dac_next_units_) {
                u.build();
            }
        }
        
        void read(std::istream &is) override {
            ArrayFSATail::read(is);
            element_size_ = 1 + 1;
            for (auto i = 0; i < next_size_ - 1; i++) {
                DacUnit unit;
                unit.read(is);
                dac_next_units_.push_back(unit);
            }
        }
        
        void write(std::ostream &os) const override {
            ArrayFSATail::write(os);
            for (auto &u : dac_next_units_) {
                u.write(os);
            }
        }
        
        size_t size_in_bytes() const override {
            auto dacSize = 0;
            for (auto u : dac_next_units_) {
                dacSize += u.size_in_bytes();
            }
            return ArrayFSATail::size_in_bytes() + dacSize;
        }
        
        void show_stat(std::ostream& os) const override {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << get_num_trans() << endl;
            os << "#elems: " << get_num_elements() << endl;
            os << "size:   " << size_in_bytes() << endl;
            os << "size bytes:   " << size_vec(bytes_) << endl;
            auto dacNextSize = 0;
            for (auto &u : dac_next_units_) {
                dacNextSize += u.size_in_bytes();
            }
            os << "size dac_next_bytes:   " << dacNextSize << endl;
            auto dacCheckSize = 0;
            dacCheckSize = dac_check_unit_.size_in_bytes();
            os << "size dac_check_bytes:   " << dacCheckSize << endl;
            os << "size label:   " << size_vec(label_bytes_) << endl;
        }
        
    private:
        std::vector<DacUnit> dac_next_units_;
        
        uint8_t get_check_(size_t trans) const override {
            return bytes_[offset_(trans) + 1];
        }
        
    };
    
}

#endif /* ArrayFSATailDAC_hpp */

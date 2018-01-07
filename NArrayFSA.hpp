//
//  NArrayFSA.hpp
//  bench
//
//  Created by 松本拓真 on 2018/01/05.
//

#ifndef NArrayFSA_hpp
#define NArrayFSA_hpp

#include "ByteData.hpp"

#include "FitValuesArray.hpp"

namespace array_fsa {
    
    class ArrayFSABuilder;
    
    class NArrayFSA : ByteData {
        friend class ArrayFSABuilder;
    public:
        using Builder = ArrayFSABuilder;
        
        // MARK: Constructor
        
        NArrayFSA() = default;
        ~NArrayFSA() = default;
        
        NArrayFSA(NArrayFSA&& rhs) noexcept : NArrayFSA() {
            this->swap(rhs);
        }
        NArrayFSA& operator=(NArrayFSA&& rhs) noexcept {
            this->swap(rhs);
            return *this;
        }
        
        NArrayFSA(const NArrayFSA&) = delete;
        NArrayFSA& operator=(const NArrayFSA&) = delete;
        
        // MARK: - Function
        
        static std::string name() {
            return "NArrayFSA";
        }
        
        bool isMember(const std::string &str) const {
            size_t edge = 0;
            for (uint8_t c : str) {
                edge = getTargetState(edge) ^ c;
                if (c != getCheck(edge)) return false;
            }
            return isFinal(edge);
        }
        
        void set_num_trans_(size_t num) {
            num_trans_ = num;
        }
        
        // MARK: Double-array
        
        size_t getTargetState(size_t index) const {
            return index ^ getNext(index);
        }
        
        size_t getNext(size_t index) const {
            return byte_array_.getValue<size_t>(index, 0) >> 1;
        }
        
        uint8_t getCheck(size_t index) const {
            return byte_array_.getValue<uint8_t>(index, 1);
        }
        
        bool isFinal(size_t index) const {
            return (byte_array_.getValue<uint8_t>(index, 0) & 1) != 0;
        }
        
        void setNext(size_t index, size_t next) {
            auto byte = next << 1 | isFinal(index);
            byte_array_.setValue(index, 0, byte);
            
            if (getNext(index) != next) { // Test
                std::cout << "Error: setNext " << index << std::endl;
            }
        }
        
        void setCheck(size_t index, uint8_t check) {
            byte_array_.setValue(index, 1, check);
            
            if (getCheck(index) != check) { // Test
                std::cout << "Error: setCheck " << index << std::endl;
            }
        }
        
        void setIsFinal(size_t index, bool isFinal) {
            uint8_t byte = byte_array_.getValue<uint8_t>(index, 0);
            if (isFinal)
                byte |= 1;
            else
                byte &= ~1;
            byte_array_.setValue(index, 0, byte);
        }
        
        void setNextAndFinal(size_t index, size_t next, bool isFinal) {
            auto value = next << 1 | isFinal;
            byte_array_.setValue(index, 0, value);
        }
        
        // MARK: Information
        
        void show_stat(std::ostream &os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << num_trans_ << endl;
            os << "#elems: " << byte_array_.numElements() << endl;
            os << "size:   " << sizeInBytes() << endl;
            os << "size bytes_:   " << byte_array_.sizeBytes() << endl;
        }
        
        size_t sizeInBytes() const override {
            auto size = byte_array_.sizeInBytes();
            size += sizeof(num_trans_);
            return size;
        }
        
        void write(std::ostream &os) const override {
            byte_array_.write(os);
            write_val(num_trans_, os);
        }
        
        void read(std::istream &is) override {
            byte_array_.read(is);
            num_trans_ = read_val<size_t>(is);
        }
        
        void swap(NArrayFSA &rhs) {
            byte_array_.swap(rhs.byte_array_);
            std::swap(num_trans_, rhs.num_trans_);
        }
        
    private:
        FitValuesArray byte_array_;
        size_t num_trans_;
        
    };
    
}

#endif /* NArrayFSA_hpp */

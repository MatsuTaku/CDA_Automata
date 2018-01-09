//
//  NArrayFSADACs.hpp
//  bench
//
//  Created by 松本拓真 on 2018/01/07.
//

#ifndef NArrayFSADACs_hpp
#define NArrayFSADACs_hpp

#include "ByteData.hpp"

#include "ArrayFSABuilder.hpp"
#include "FitValuesArray.hpp"
#include "DACs.hpp"
#include "Rank.hpp"

namespace array_fsa {
    
    class NArrayFSADACs : ByteData {
        friend class ArrayFSABuilder;
    public:
        // MARK: Constructor
        
        NArrayFSADACs() = default;
        ~NArrayFSADACs() = default;
        
        NArrayFSADACs(NArrayFSADACs&& rhs) noexcept : NArrayFSADACs() {
            this->swap(rhs);
        }
        NArrayFSADACs& operator=(NArrayFSADACs&& rhs) noexcept {
            this->swap(rhs);
            return *this;
        }
        
        NArrayFSADACs(const NArrayFSADACs&) = delete;
        NArrayFSADACs& operator=(const NArrayFSADACs&) = delete;
        
        // MARK: - Function
        
        static NArrayFSADACs build(PlainFSA &origFsa) {
            return ArrayFSABuilder::buildNArrayFSADACs(origFsa);
        }
        
        static std::string name() {
            return "NArrayFSADACs";
        }
        
        bool isMember(const std::string& str) const {
            auto edge = 0;
            for (uint8_t c : str) {
                edge = getTargetState(edge) ^ c;
                if (c != getCheck(edge)) return false;
            }
            return isFinal(edge);
        }
        
        void set_num_trans_(size_t num) {
            num_trans_ = num;
        }
        
        void setValuesSizes(size_t nextSize, size_t checkSize) {
            byte_array_.insertValueSize(0, nextSize);
            byte_array_.insertValueSize(1, checkSize);
        }
        
        // MARK: Double-array
        
        size_t getTargetState(size_t index) const {
            return index ^ getNext(index);
        }
        
        // MARK: - DACs
        
        size_t getNext(size_t index) const {
            auto next = byte_array_.getValue<size_t>(index, 0);
            return next | flows_.getValue(index) << 8;
        }
        
        void setNext(size_t index, size_t next) {
            byte_array_.setValue(index, 0, next & 0xff);
            auto flow = next >> 8;
            if (flow != 0) {
                flows_.setValue(index, flow);
            }
            
//            if (getNext(index) != next) { // Test
//                std::cout << "Error: setNext " << index << std::endl;
//            }
        }
        
        // MARK: -
        
        uint8_t getCheck(size_t index) const {
            return byte_array_.getValue<uint8_t>(index, 1);
        }
        
        void setCheck(size_t index, uint8_t check) {
            byte_array_.setValue(index, 1, check);
            
            if (getCheck(index) != check) { // Test
                std::cout << "Error: setCheck " << index << std::endl;
            }
        }
        
        bool isFinal(size_t index) const {
            return final_bits_.get(index);
        }
        
        void setIsFinal(size_t index, bool isFinal) {
            final_bits_.set(index, isFinal);
        }
        
        // MARK: Information
        
        void show_stat(std::ostream &os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << num_trans_ << endl;
            os << "#elems: " << byte_array_.numElements() << endl;
            os << "size:   " << sizeInBytes() << endl;
            os << "size bytes_:   " << byte_array_.sizeBytes() << endl;
            os << "size flows_:   " << flows_.sizeInBytes() << endl;
            os << "size final_bits_:" << final_bits_.size_in_bytes() << endl;
        }
        
        size_t sizeInBytes() const override {
            auto size = byte_array_.sizeInBytes();
            size += sizeof(num_trans_);
            size += flows_.sizeInBytes();
            size += final_bits_.size_in_bytes();
            return size;
        }
        
        void write(std::ostream &os) const override {
            byte_array_.write(os);
            write_val(num_trans_, os);
            flows_.write(os);
            final_bits_.write(os);
        }
        
        void read(std::istream &is) override {
            byte_array_.read(is);
            num_trans_ = read_val<size_t>(is);
            flows_.read(is);
            final_bits_.read(is);
        }
        
        void swap(NArrayFSADACs &rhs) {
            byte_array_.swap(rhs.byte_array_);
            std::swap(num_trans_, rhs.num_trans_);
            flows_.swap(rhs.flows_);
            final_bits_.swap(rhs.final_bits_);
        }
        
    private:
        FitValuesArray byte_array_;
        size_t num_trans_ = 0;
        DACs flows_;
        Rank final_bits_;
        
    };
    
}

#endif /* NArrayFSADACs_hpp */

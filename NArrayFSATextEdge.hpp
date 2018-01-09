//
//  NArrayFSATextEdge.hpp
//  bench
//
//  Created by 松本拓真 on 2018/01/07.
//

#ifndef NArrayFSATextEdge_hpp
#define NArrayFSATextEdge_hpp

#include "ByteData.hpp"

#include "ArrayFSATailBuilder.hpp"
#include "FitValuesArray.hpp"
#include "Rank.hpp"
#include "DACs.hpp"

namespace array_fsa {
    
    class NArrayFSATextEdge : ByteData {
        friend class ArrayFSATailBuilder;
    public:
        // MARK: Constructor
        
        NArrayFSATextEdge() = default;
        ~NArrayFSATextEdge() = default;
        
        NArrayFSATextEdge(NArrayFSATextEdge&& rhs) noexcept : NArrayFSATextEdge() {
            this->swap(rhs);
        }
        NArrayFSATextEdge& operator=(NArrayFSATextEdge&& rhs) noexcept {
            this->swap(rhs);
            return *this;
        }
        
        NArrayFSATextEdge(const NArrayFSATextEdge&) = delete;
        NArrayFSATextEdge& operator=(const NArrayFSATextEdge&) = delete;
        
        // MARK: - Function
        
        static NArrayFSATextEdge build(PlainFSA &origFsa) {
            return ArrayFSATailBuilder::buildNArrayFSATextEdge(origFsa);
        }
        
        static std::string name() {
            return "NArrayFSATextEdge";
        }
        
        bool isMember(const std::string& str) const {
            auto edge = 0;
            for (auto begin = 0; begin < str.size();) {
                uint8_t symbol = str[begin];
                edge = getTargetState(edge) ^ symbol;
                if (!hasLabel(edge)) {
                    if (symbol != getCheck(edge)) return false;
                    begin++;
                } else {
                    auto labelIndex = getLabelIndex(edge);
                    auto c = labels_[labelIndex];
                    while (symbol == c) {
                        c = labels_[++labelIndex];
                        if (++begin >= str.size()) break;
                        symbol = str[begin];
                    }
                    if (c != '\0') return false;
                }
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
        
        void setNextAndFinal(size_t index, size_t next, bool isFinal) {
            auto value = next << 1 | isFinal;
            byte_array_.setValue(index, 0, value);
        }
        
        size_t getTargetState(size_t index) const {
            return index ^ getNext(index);
        }
        
        size_t getNext(size_t index) const {
            return byte_array_.getValue<size_t>(index, 0) >> 1;
        }
        
        void setNext(size_t index, size_t next) {
            auto byte = next << 1 | isFinal(index);
            byte_array_.setValue(index, 0, byte);
            
            if (getNext(index) != next) { // Test
                std::cout << "Error: setNext " << index << std::endl;
            }
        }
        
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
            return (byte_array_.getValue<uint8_t>(index, 0) & 1) != 0;
        }
        
        void setIsFinal(size_t index, bool isFinal) {
            uint8_t byte = byte_array_.getValue<uint8_t>(index, 0);
            if (isFinal)
                byte |= 1;
            else
                byte &= ~1;
            byte_array_.setValue(index, 0, byte);
        }
        
        // MARK: - label index
        
        bool hasLabel(size_t index) const {
//            return has_label_bits_.get(index);
            return label_index_flows_.units_[0].getBit(index);
        }
        
        void setHasLabel(size_t index, bool hasLabel) {
//            has_label_bits_.set(index, hasLabel);
            label_index_flows_.units_[0].setBit(index, hasLabel);
        }
        
        size_t getLabelIndex(size_t index) const {
            auto labelIndex = getCheck(index);
            return labelIndex | label_index_flows_.getValue(index) << 8;
        }
        
        void setLabelIndex(size_t index, size_t labelIndex) {
            setCheck(index, labelIndex & 0xff);
            label_index_flows_.setValue(index, labelIndex >> 8);
        }
        
        // MARK: -
        
        // MARK: Information
        
        void show_stat(std::ostream &os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << num_trans_ << endl;
            os << "#elems: " << byte_array_.numElements() << endl;
            os << "size:   " << sizeInBytes() << endl;
            os << "size bytes_:   " << byte_array_.sizeBytes() << endl;
            os << "size labels_:  " << size_vec(labels_) << endl;
            os << "size has_label_bits_: " << has_label_bits_.size_in_bytes() << endl;
            os << "size label_index_flows_: " << label_index_flows_.sizeInBytes() << endl;
        }
        
        size_t sizeInBytes() const override {
            auto size = byte_array_.sizeInBytes();
            size += sizeof(num_trans_);
            size += size_vec(labels_);
            size += has_label_bits_.size_in_bytes();
            size += label_index_flows_.sizeInBytes();
            return size;
        }
        
        void write(std::ostream &os) const override {
            byte_array_.write(os);
            write_val(num_trans_, os);
            write_vec(labels_, os);
            has_label_bits_.write(os);
            label_index_flows_.write(os);
        }
        
        void read(std::istream &is) override {
            byte_array_.read(is);
            num_trans_ = read_val<size_t>(is);
            labels_ = read_vec<uint8_t>(is);
            has_label_bits_.read(is);
            label_index_flows_.read(is);
        }
        
        void swap(NArrayFSATextEdge &rhs) {
            byte_array_.swap(rhs.byte_array_);
            std::swap(num_trans_, rhs.num_trans_);
            labels_.swap(rhs.labels_);
            has_label_bits_.swap(rhs.has_label_bits_);
            label_index_flows_.swap(rhs.label_index_flows_);
        }
        
    private:
        FitValuesArray byte_array_;
        size_t num_trans_ = 0;
        std::vector<uint8_t> labels_;
        Rank has_label_bits_;
        DACs label_index_flows_;
        
    };
    
}

#endif /* NArrayFSATextEdge_hpp *///


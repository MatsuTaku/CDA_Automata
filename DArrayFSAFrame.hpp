//
//  DArrayFSAFrame.hpp
//  bench
//
//  Created by 松本拓真 on 2017/12/06.
//

#ifndef DArrayFSAFrame_hpp
#define DArrayFSAFrame_hpp

#include "basic.hpp"

#include "Calc.hpp"
#include "Accessory.hpp"

namespace double_array {
    
    class DArrayFSAFrame {
        friend class NextAccessory;
        friend class CheckAccessory;
        friend class FinalAccessory;
        
    public:
        using byteVector = std::vector<uint8_t>;
        
        DArrayFSAFrame() = default;
        virtual ~DArrayFSAFrame() = default;
        
        DArrayFSAFrame(DArrayFSAFrame &&rhs) noexcept : DArrayFSAFrame() {
            this->swap(rhs);
        }
        
        DArrayFSAFrame & operator=(DArrayFSAFrame &&rhs) noexcept {
            this->swap(rhs);
            return *this;
        }
        
        DArrayFSAFrame(const DArrayFSAFrame &) = delete;
        DArrayFSAFrame & operator=(const DArrayFSAFrame &) = delete;
        
        // MARK: - set getter setter
        
        void setNextAccessory(Accessory<size_t> accessory) {
            next_ = accessory;
        }
        
        void setCheckAccessory(Accessory<uint8_t> accessory) {
            check_ = accessory;
        }
        
        void setFinalAccessory(Accessory<bool> accessory) {
            is_final_ = accessory;
        }
        
        void setLookupAccessory(Scout accessory) {
            lookup_ = accessory;
        }
        
        // MARK: - getter
        
        size_t getNext(size_t index) const {
            return next_.get(*this, index);
        }
        
        uint8_t getCheck(size_t index) const {
            return check_.get(*this, index);
        }
        
        bool getIsFinal(size_t index) const {
            return is_final_.get(*this, index);
        }
        
        bool lookup(const std::string &str) const {
            return lookup_.scouts(*this, str);
        }
        
        // MARK: - setter
        
        void setNext(size_t index, size_t next) {
            next_.set(*this, index, next);
        }
        
        void setCheck(size_t index, uint8_t check) {
            check_.set(*this, index, check);
        }
        
        void setIsFinal(size_t index, bool isFinal) {
            is_final_.set(*this, index, isFinal);
        }
        
        // MARK: - function
        
        std::string name() const {
            return name_;
        }
        
        size_t offset(size_t index) const {
            return index * element_size_;
        }
        
        size_t nextOffset(size_t index) const {
            return offset(index);
        }
        
        size_t checkOffset(size_t index) const {
            return offset(index) + next_size_;
        }
        
        size_t getTargetState(size_t index) const {
            return index ^ getNext(index);
        }
        
        void setName(const std::string &name) {
            name_ = name;
        }
        
        void resizeBytes(size_t size) {
            bytes_.resize(size);
        }
        
        void calcNextSize(size_t numElems) {
            next_size_ = Calc::sizeFitInBytes(numElems);
        }
        
        size_t getNextSize() const {
            return next_size_;
        }
        
        size_t getNumElements() const {
            return bytes_.size() / element_size_;
        }
        
        size_t getElementSize() const {
            return element_size_;
        }
        
        void setNumTrans(size_t numTrans) {
            num_trans_ = numTrans;
        }
        
        void setElementSize(size_t size) {
            element_size_ = size;
        }
        
        // MARK: - file access
        
        virtual void read(std::istream &is) {
            using namespace array_fsa;
            bytes_ = read_vec<uint8_t>(is);
            next_size_ = read_val<size_t>(is);
            label_index_size_ = read_val<size_t>(is);
            element_size_ = next_size_ + 1;
            num_trans_ = read_val<size_t>(is);
        }
        
        virtual void write(std::ostream &os) const {
            using namespace array_fsa;
            write_vec(bytes_, os);
            write_val(next_size_, os);
            write_val(label_index_size_, os);
            write_val(num_trans_, os);
        }
        
        void swap(DArrayFSAFrame &rhs) {
            using std::swap;
            bytes_.swap(rhs.bytes_);
            swap(next_size_, rhs.next_size_);
            swap(element_size_, rhs.element_size_);
            swap(label_index_size_, rhs.label_index_size_);
            swap(num_trans_, rhs.num_trans_);
        }
        
        virtual size_t sizeInBytes() const {
            auto size = array_fsa::size_vec(bytes_);
            size += sizeof(next_size_);
            size += sizeof(label_index_size_);
            size += sizeof(num_trans_);
            return size;
        }
        
    protected:
        std::string name_;
        
        byteVector bytes_;
        size_t next_size_ = 0;
        const size_t kCheckSize = 1;
        size_t label_index_size_ = 0;
        size_t element_size_ = 0; // from calculation
        size_t num_trans_ = 0;
        
        Accessory<size_t> next_;
        Accessory<uint8_t> check_;
        Accessory<bool> is_final_;
        
        Scout lookup_;
        
    };
    
}

#endif /* DArrayFSAFrame_hpp */

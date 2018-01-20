//
//  NextCheck.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/13.
//

#ifndef NextCheck_hpp
#define NextCheck_hpp

#include "ByteData.hpp"

#include "FitValuesArray.hpp"
#include "DACs.hpp"

#include "Calc.hpp"
#include "basic.hpp"

namespace array_fsa {
    
    struct Id : ByteData {
        DACs flows;
        bool useDac = false;
        
        size_t sizeInBytes() const override {
            auto size = 1;
            size += flows.sizeInBytes();
            size += 1;
            return size;
        }
        
        void write(std::ostream& os) const override {
            flows.write(os);
            write_val(static_cast<uint8_t>(useDac), os);
        }
        
        void read(std::istream& is) override {
            flows.read(is);
            useDac = static_cast<bool>(read_val<uint8_t>(is));
        }
    };
    
    class NextCheck : ByteData {
    public:
        NextCheck() = default;
        ~NextCheck() = default;
        
        size_t next(size_t index) const {
            auto next = bytes_.getValue<size_t>(index, 0);
            if (!nextId.useDac) return next;
            return next | nextId.flows[index] << 8;
        }
        
        size_t check(size_t index) const {
            auto check = bytes_.getValue<uint8_t>(index, 1);
            if (!checkId.useDac) return check;
            auto flow = checkId.flows[index];
            return check | flow << 8;
        }
        
        size_t numElements() const {
            return bytes_.numElements();
        }
        
        bool getBitInFlow(size_t index) const {
            if (!checkId.useDac) abort();
            return checkId.flows.getBitInFirstUnit(index);
        }
        
        // MARK: - build
        
        void setNext(size_t index, size_t next) {
            bytes_.setValue(index, 0, next);
            if (!nextId.useDac) return;
            nextId.flows.setValue(index, next >> 8);
        }
        
        void setCheck(size_t index, uint8_t check) {
            bytes_.setValue(index, 1, check);
        }
        
        void setStringIndex(size_t index, size_t strIndex) {
            if (!checkId.useDac) return;
            setCheck(index, strIndex & 0xff);
            checkId.flows.setValue(index, strIndex >> 8);
        }
        
        void setBitInFlow(size_t index, bool bit) {
            if (!checkId.useDac) abort();
            checkId.flows.setBitInFirstUnit(index, bit);
        }
        
        // MARK: - Protocol settings
        
        // No.1 optional
        void setUseDacNext(bool use) {
            nextId.useDac = use;
        }
        
        // No.2 optional
        void setUseDacCheck(bool use) {
            checkId.useDac = use;
        }
        
        // No.3
        void setNumElement(size_t num, bool bitInto) {
            auto nextSize = Calc::sizeFitInBytes(bitInto ? num << 1 : num);
            std::vector<size_t> sizes = { nextId.useDac ? 1 : nextSize, 1 };
            bytes_.setValueSizes(sizes);
            bytes_.resize(num);
            if (!nextId.useDac) return;
            nextId.flows.setMaxValue(num << (bitInto ? 1 : 0) >> 8);
        }
        
        // No.4 if use dac check
        void setNumStrings(size_t num) {
            if (!checkId.useDac) return;
            checkId.flows.useLink(true);
            auto size = Calc::sizeFitInBytes(num - 1);
            checkId.flows.setUnitSize(size > 1 ? size - 1 : 1);
            checkId.flows.setMaxValue((num - 1) >> 8);
        }
        
        // Finaly. If use dac
        void buildBitArray() {
            if (nextId.useDac) nextId.flows.build();
            if (checkId.useDac) checkId.flows.build();
        }
        
        // MARK: - ByteData methods
        
        size_t sizeInBytes() const override {
            auto size = bytes_.sizeInBytes();
            size += nextId.sizeInBytes();
            size += checkId.sizeInBytes();
            return size;
        }
        void write(std::ostream& os) const override {
            bytes_.write(os);
            nextId.write(os);
            checkId.write(os);
        }
        void read(std::istream& is) override {
            bytes_.read(is);
            nextId.read(is);
            checkId.read(is);
        }
        
        void showStatus(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << "NextCheck" << " ---" << endl;
            os << "size:   " << sizeInBytes() << endl;
            os << "size bytes:   " << bytes_.sizeInBytes() << endl;
            os << "size next flow:   " << nextId.flows.sizeInBytes() << endl;
            os << "size check flow:   " << checkId.flows.sizeInBytes() << endl;
            if (!nextId.useDac)
                showSizeMap(os);
        }
        
        void showSizeMap(std::ostream &os) const {
            auto numElem = numElements();
            std::vector<size_t> nexts(numElem);
            for (auto i = 0; i < numElem; i++)
                nexts[i] = bytes_.getValue<size_t>(i, 0);
            
            auto showList = [&](const std::vector<size_t> &list) {
                using std::endl;
                os << "-- " << "Next Map" << " --" << endl;
                for (auto c : list)
                    os << c << "\t" << endl;
                os << "/ " << numElem << endl;
            };
            auto counts = Calc::separateCountsInSizeOf(nexts);
            showList(counts);
            auto xorCounts = Calc::separateCountsInXorSizeOf(nexts);
            showList(xorCounts);
        }
        
        // MARK: - Copy guard
        
        NextCheck (const NextCheck&) = delete;
        NextCheck& operator =(const NextCheck&) = delete;
        
        NextCheck (NextCheck&&) noexcept = default;
        NextCheck& operator =(NextCheck&&) noexcept = default;
        
    private:
        FitValuesArray bytes_;
        Id nextId;
        Id checkId;
        
    };
    
}

#endif /* NextCheck_hpp */

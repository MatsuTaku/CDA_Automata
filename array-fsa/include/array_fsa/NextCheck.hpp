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

#include "sim_ds/DACs.hpp"
#include "sim_ds/Calc.hpp"

namespace array_fsa {
    
    template<bool N, bool C>
    class NextCheck : ByteData {
    public:
        static constexpr bool useNextCodes = N;
        static constexpr bool useCheckCodes = C;
        
    private:
        static constexpr size_t kFixedBits = 8;
        
    public:
        NextCheck() = default;
        ~NextCheck() = default;
        
        size_t next(size_t index) const {
            auto next = elements_.getValue<size_t>(index, 0);
            if (!N) return next;
            if (!nextLinkBits_[index])
                return next;
            else
                return next | (nextFlow[nextLinkBits_.rank(index)] << kFixedBits);
        }
        
        uint8_t check(size_t index) const {
            return elements_.getValue<uint8_t>(index, 1);
        }
        
        size_t stringId(size_t index) const {
            assert(C);
            
            auto id = check(index);
            if (!checkLinkBits_[index])
                return id;
            else
                return id | (checkFlow[checkLinkBits_.rank(index)] << kFixedBits);
        }
        
        size_t numElements() const {
            return elements_.numElements();
        }
        
        // MARK: - build
        
        void setNext(size_t index, size_t next) {
            elements_.setValue(index, 0, next);
            if (!N) return;
            auto flow = next >> kFixedBits;
            if (flow > 0) {
                nextLinkBits_.set(index, true);
                nextFlow.setValue(numNextLinks_++, flow);
            }
        }
        
        void setCheck(size_t index, uint8_t check) {
            elements_.setValue(index, 1, check);
        }
        
        void setStringIndex(size_t index, size_t strIndex) {
            assert(C);
            
            setCheck(index, strIndex & 0xff);
            auto flow = strIndex >> kFixedBits;
            if (flow > 0) {
                checkLinkBits_.set(index, true);
                checkFlow.setValue(numCheckLinks_++, flow);
            }
        }
        
        // MARK: - Protocol settings
        
        // No.1
        void setNumElement(size_t num, bool bitInto) {
            auto nextSize = sim_ds::Calc::sizeFitInBytes(bitInto ? num << 1 : num);
            std::vector<size_t> sizes = { N ? 1 : nextSize, 1 };
            elements_.setValueSizes(sizes);
            elements_.resize(num);
        }
        
//        // No.2 if use dac check
//        void setNumStrings(size_t num) {
//            assert(C);
//            auto maxSize = sim_ds::Calc::sizeFitInBytes(num - 1);
//            auto cCodesName = typeid(cCodes).name();
//            if (cCodesName == typeid(DACs<true>).name() ||
//                cCodesName == typeid(DACs<false>).name())
//                checkFlow.setUnitSize(std::max(maxSize - 1, size_t(1)));
//        }
        
        // Finaly. If use dac
        void buildBitArray() {
            if (N) {
                nextLinkBits_.build();
                nextFlow.build();
            }
            if (C) {
                checkFlow.build();
                checkLinkBits_.build();
            }
        }
        
        // MARK: - ByteData methods
        
        size_t sizeInBytes() const override {
            auto size = elements_.sizeInBytes();
            if (N) {
                size += nextLinkBits_.sizeInBytes();
                size += nextFlow.sizeInBytes();
            }
            if (C) {
                size += checkLinkBits_.sizeInBytes();
                size += checkFlow.sizeInBytes();
            }
            return size;
        }
        void write(std::ostream& os) const override {
            elements_.write(os);
            if (N) {
                nextLinkBits_.write(os);
                nextFlow.write(os);
            }
            if (C) {
                checkLinkBits_.write(os);
                checkFlow.write(os);
            }
        }
        void read(std::istream& is) override {
            elements_.read(is);
            if (N) {
                nextLinkBits_.read(is);
                nextFlow.read(is);
            }
            if (C) {
                checkLinkBits_.read(is);
                checkFlow.read(is);
            }
        }
        
        void showStatus(std::ostream &os) const;
        void showSizeMap(std::ostream &os) const;
        
        // MARK: - Copy guard
        
        NextCheck (const NextCheck&) = delete;
        NextCheck& operator =(const NextCheck&) = delete;
        
        NextCheck (NextCheck&&) noexcept = default;
        NextCheck& operator =(NextCheck&&) noexcept = default;
        
    private:
        FitValuesArray elements_;
        size_t numNextLinks_ = 0;
        size_t numCheckLinks_ = 0;
        sim_ds::BitVector nextLinkBits_;
        sim_ds::BitVector checkLinkBits_;
        sim_ds::DACs nextFlow;
        sim_ds::DACs checkFlow;
        
    };
    
    
    template<bool N, bool C>
    void NextCheck<N, C>::showStatus(std::ostream &os) const {
        using std::endl;
        auto codesName = [=](bool use) {
            return use ? "DACs" : "Plain";
        };
        os << "--- Stat of " << "NextCheck " << codesName(useNextCodes) << "|" << codesName(useCheckCodes) << " ---" << endl;
        os << "size:   " << sizeInBytes() << endl;
        os << "size bytes:   " << elements_.sizeInBytes() << endl;
        os << "size next flow:   " << nextFlow.sizeInBytes() << endl;
        os << "size check flow:   " << checkFlow.sizeInBytes() << endl;
        nextFlow.showStatus(os);
        checkFlow.showStatus(os);
        showSizeMap(os);
    }
    
    template<bool N, bool C>
    void NextCheck<N, C>::showSizeMap(std::ostream &os) const {
        auto numElem = numElements();
        std::vector<size_t> nexts(numElem);
        for (auto i = 0; i < numElem; i++)
            nexts[i] = next(i) >> (!N ? 1 : 0);
        
        auto showList = [&](const std::vector<size_t> &list) {
            using std::endl;
            os << "-- " << "Next Map" << " --" << endl;
            for (auto c : list)
                os << c << "\t" << endl;
            os << "/ " << numElem << endl;
        };
        auto counts = sim_ds::Calc::separateCountsInSizeOf(nexts);
        showList(counts);
        auto xorCounts = sim_ds::Calc::separateCountsInXorSizeOf(nexts);
        showList(xorCounts);
    }
}

#endif /* NextCheck_hpp */

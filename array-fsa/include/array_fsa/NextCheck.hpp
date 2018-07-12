//
//  NextCheck.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/13.
//

#ifndef NextCheck_hpp
#define NextCheck_hpp

#include "ByteData.hpp"

#include "MultipleVector.hpp"

#include "sim_ds/DacVector.hpp"
#include "sim_ds/Calc.hpp"

namespace array_fsa {
    
    template<bool N, bool C, bool INCLUDE_VALUE>
    class NextCheck : ByteData {
    public:
        static constexpr bool useNextCodes = N;
        static constexpr bool useCheckCodes = C;
        
    private:
        static constexpr size_t kFixedBits = 8;
        enum ElementNumber {
            ENNext = 0,
            ENCheck = 1,
            ENStore = 2,
            ENAccStore = 3,
        };
        
    public:
        NextCheck() = default;
        ~NextCheck() = default;
        
        // MARK: - Copy guard
        
        NextCheck (const NextCheck&) = delete;
        NextCheck& operator =(const NextCheck&) = delete;
        
        NextCheck (NextCheck&&) noexcept = default;
        NextCheck& operator =(NextCheck&&) noexcept = default;
        
    public:
        size_t next(size_t index) const {
            auto next = elements_.get<ENNext, size_t>(index);
            if constexpr (!N) return next;
            
            if (!nextLinkBits_[index])
                return next;
            else {
                auto rank = nextLinkBits_.rank(index);
                return next | (nextFlow[rank] << kFixedBits);
            }
        }
        
        uint8_t check(size_t index) const {
            return elements_.get<ENCheck, uint8_t>(index);
        }
        
        size_t stringId(size_t index) const {
            assert(C);
            
            auto id = check(index);
            if (!checkLinkBits_[index])
                return id;
            else {
                auto rank = checkLinkBits_.rank(index);
                return id | (checkFlow[rank] << kFixedBits);
            }
        }
        
        size_t numElements() const {
            return elements_.size();
        }
        
        // MARK: - build
        
        void setNext(size_t index, size_t next) {
            elements_.set<ENNext>(index, next);
            
            if constexpr (!N) return;
            auto flow = next >> kFixedBits;
            bool hasFlow = flow > 0;
            nextLinkBits_.set(index, hasFlow);
            if (hasFlow)
                nextFlow.set(numNextLinks_++, flow);
        }
        
        void setCheck(size_t index, uint8_t check) {
            elements_.set<ENCheck>(index, check);
        }
        
        void setStringId(size_t index, size_t strIndex) {
            assert(C);
            
            setCheck(index, strIndex & 0xff);
            auto flow = strIndex >> kFixedBits;
            bool hasFlow = flow > 0;
            checkLinkBits_.set(index, hasFlow);
            if (hasFlow)
                checkFlow.set(numCheckLinks_++, flow);
        }
        
        void setStore(size_t index, size_t store) {
            assert(INCLUDE_VALUE);
            
            elements_.set<ENStore>(index, store);
        }
        
        void setAccStore(size_t index, size_t as) {
            assert(INCLUDE_VALUE);
            
            elements_.set<ENAccStore>(index, as);
        }
        
        // MARK: - Protocol settings
        
        // First. Set size of elements
        void resize(size_t size, size_t words = 0) {
            auto bitInto = !useNextCodes;
            auto nextSize = sim_ds::Calc::sizeFitInBytes(bitInto ? (size << 1) : size);
            std::vector<size_t> sizes = { N ? 1 : nextSize, 1 };
            if constexpr (INCLUDE_VALUE) {
                auto storeSize = sim_ds::Calc::sizeFitInBytes(words);
                sizes.push_back(storeSize); // Store
                sizes.push_back(storeSize); // AccumulateStore
            }
            elements_.setValueSizes(sizes);
            elements_.resize(size);
        }
        
        // Finaly. If use dac
        void buildBitArray() {
            if constexpr (N) {
                nextLinkBits_.build();
                nextFlow.build();
            }
            if constexpr (C) {
                checkLinkBits_.build();
                checkFlow.build();
            }
        }
        
        // MARK: - ByteData methods
        
        size_t sizeInBytes() const override {
            auto size = elements_.sizeInBytes();
            if constexpr (N) {
                size += nextLinkBits_.sizeInBytes();
                size += nextFlow.sizeInBytes();
            }
            if constexpr (C) {
                size += checkLinkBits_.sizeInBytes();
                size += checkFlow.sizeInBytes();
            }
            return size;
        }
        
        void write(std::ostream& os) const override {
            elements_.write(os);
            if constexpr (N) {
                nextLinkBits_.write(os);
                nextFlow.write(os);
            }
            if constexpr (C) {
                checkLinkBits_.write(os);
                checkFlow.write(os);
            }
        }
        
        void read(std::istream& is) override {
            elements_.read(is);
            if constexpr (N) {
                nextLinkBits_.read(is);
                nextFlow.read(is);
            }
            if constexpr (C) {
                checkLinkBits_.read(is);
                checkFlow.read(is);
            }
        }
        
        void showStatus(std::ostream &os) const;
        void showSizeMap(std::ostream &os) const;
        
    private:
        MultipleVector elements_;
        // Enabled at NextCheck<true, C>
        size_t numNextLinks_ = 0;
        sim_ds::BitVector nextLinkBits_;
        sim_ds::DacVector nextFlow;
        // Enabled at NextCheck<N, true>
        size_t numCheckLinks_ = 0;
        sim_ds::BitVector checkLinkBits_;
        sim_ds::DacVector checkFlow;
        
    };
    
    
    template<bool N, bool C, bool V>
    void NextCheck<N, C, V>::showStatus(std::ostream &os) const {
        using std::endl;
        auto codesName = [=](bool use) {
            return use ? "DACs" : "Plain";
        };
        os << "--- Stat of " << "NextCheck " << codesName(useNextCodes) << "|" << codesName(useCheckCodes) << " ---" << endl;
        os << "size:   " << sizeInBytes() << endl;
        os << "size bytes:   " << elements_.sizeInBytes() << endl;
        if constexpr (N) {
            os << "size next flow:   " << nextFlow.sizeInBytes() << endl;
            nextFlow.showStatus(os);
            os << "---  ---" << endl;
        }
        if constexpr (C) {
            os << "size check flow:   " << checkFlow.sizeInBytes() << endl;
            checkFlow.showStatus(os);
            os << "---  ---" << endl;
        }
        showSizeMap(os);
    }
    
    template<bool N, bool C, bool V>
    void NextCheck<N, C, V>::showSizeMap(std::ostream &os) const {
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

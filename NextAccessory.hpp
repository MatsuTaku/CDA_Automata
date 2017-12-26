//
//  NextAccessory.hpp
//  double_array
//
//  Created by 松本拓真 on 2017/12/06.
//

#ifndef NextAccessory_hpp
#define NextAccessory_hpp

#include "Accessory.hpp"

#include "DArrayFSAFrame.hpp"

namespace double_array {
    
    enum class NextAccessoryType {
        Plain,
        DAC,
    };
    
    class NextAccessory : public Accessory<size_t> {
    public:
        NextAccessory(NextAccessoryType type);
        void initPlain();
    };
    
    // MARK: - inline function
    
    inline NextAccessory::NextAccessory(NextAccessoryType type) {
        switch (type) {
            case NextAccessoryType::Plain:
                initPlain();
                break;
            case NextAccessoryType::DAC:
                break;
        }
    }
    
    inline void NextAccessory::initPlain() {
        getterType getter = [](const DArrayFSAFrame &fsa, size_t index) {
            size_t next = 0;
            std::memcpy(&next, &fsa.bytes_[fsa.nextOffset(index)], fsa.next_size_);
            return next >> 1;
        };
        setterType setter = [](DArrayFSAFrame &fsa, size_t index, size_t next) {
            next <<= 1;
            next |= fsa.getIsFinal(index);
            std::memcpy(&fsa.bytes_[fsa.nextOffset(index)], &next, fsa.next_size_);
        };
        initGetterSetter(getter, setter);
    }
    
}


#endif /* NextAccessory_hpp */

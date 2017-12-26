//
//  CheckAccessory.hpp
//  bench
//
//  Created by 松本拓真 on 2017/12/07.
//

#ifndef CheckAccessory_hpp
#define CheckAccessory_hpp

#include "Accessory.hpp"

#include "DArrayFSAFrame.hpp"

namespace double_array {
    
    enum class CheckAccessoryType {
        NonLabel,
        LabelPlain
    };
    
    class CheckAccessory : public Accessory<uint8_t> {
    public:
        CheckAccessory(CheckAccessoryType type);
        void initNonLabel();
    };
    
    // MARK: - inline function
    
    inline CheckAccessory::CheckAccessory(CheckAccessoryType type) {
        switch (type) {
            case CheckAccessoryType::NonLabel:
                initNonLabel();
                break;
            case CheckAccessoryType::LabelPlain:
                break;
        }
    }
    
    inline void CheckAccessory::initNonLabel() {
        getterType getter = [](const DArrayFSAFrame &fsa, size_t index) {
            uint8_t check = 0;
            std::memcpy(&check, &fsa.bytes_[fsa.checkOffset(index)], fsa.kCheckSize);
            return check;
        };
        setterType setter = [](DArrayFSAFrame &fsa, size_t index, uint8_t check) {
            std::memcpy(&fsa.bytes_[fsa.checkOffset(index)], &check, fsa.kCheckSize);
        };
        initGetterSetter(getter, setter);
    }
    
}

#endif /* CheckAccessory_hpp */

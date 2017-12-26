//
//  FinalAccessory.hpp
//  double_array
//
//  Created by 松本拓真 on 2017/12/10.
//

#ifndef FinalAccessory_hpp
#define FinalAccessory_hpp

#include "Accessory.hpp"

#include "DArrayFSAFrame.hpp"

namespace double_array {
    
    enum class FinalAccessoryType {
        TopNext,
        Vector
    };
    
    class FinalAccessory : public Accessory<bool> {
    public:
        FinalAccessory(FinalAccessoryType type);
        void initTopNext();
    };
    
    // MARK: - inline function
    
    inline FinalAccessory::FinalAccessory(FinalAccessoryType type) {
        switch (type) {
            case FinalAccessoryType::TopNext:
                initTopNext();
                break;
            case FinalAccessoryType::Vector:
                break;
        }
    }
    
    inline void FinalAccessory::initTopNext() {
        getterType getter = [](const DArrayFSAFrame &fsa, size_t index) {
            size_t next = 0;
            std::memcpy(&next, &fsa.bytes_[fsa.nextOffset(index)], 1);
            return (next & 1) != 0;
        };
        setterType setter = [](DArrayFSAFrame &fsa, size_t index, bool isFinal) {
            size_t next = 0;
            std::memcpy(&next, &fsa.bytes_[fsa.nextOffset(index)], 1);
            next &= 0xfe;
            next |= isFinal;
            std::memcpy(&fsa.bytes_[fsa.nextOffset(index)], &next, 1);
        };
        initGetterSetter(getter, setter);
    }
            
}

#endif /* FinalAccessory_hpp */

//
//  LookupAccessory.hpp
//  double_array
//
//  Created by 松本拓真 on 2017/12/10.
//

#ifndef LookupAccessory_hpp
#define LookupAccessory_hpp

#include "Accessory.hpp"

#include "DArrayFSAFrame.hpp"

namespace double_array {
    
    enum class LookupAccessoryType {
        Plain,
        LabelDict
    };
    
    class LookupAccessory : public Scout {
    public:
        LookupAccessory(LookupAccessoryType type);
        void setAccessoryType(LookupAccessoryType);
        void initPlain();
    };
    
    // MARK: - inline func
    
    inline LookupAccessory::LookupAccessory(LookupAccessoryType type) {
        setAccessoryType(type);
    }
    
    inline void LookupAccessory::setAccessoryType(LookupAccessoryType type) {
        switch (type) {
            case LookupAccessoryType::Plain:
                initPlain();
                break;
            case LookupAccessoryType::LabelDict:
                break;
        }
    }
    
    inline void LookupAccessory::initPlain() {
        type lookup = [](const DArrayFSAFrame &fsa, const std::string &str) {
            size_t arc = 0;
            for (uint8_t c : str) {
                arc = fsa.getTargetState(arc) ^ c;
                auto check = fsa.getCheck(arc);
                if (c != check) {
                    return false;
                }
            }
            return fsa.getIsFinal(arc);
        };
        initScouts(lookup);
    }
    
}

#endif /* LookupAccessory_hpp */

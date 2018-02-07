//
//  CodeArray.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/02/06.
//

#ifndef CodeArray_hpp
#define CodeArray_hpp

#include "DACs.hpp"
#include "SACs.hpp"

namespace array_fsa {
    
    template <class CODE_TYPE>
    class CodeArray {
    public:
        CodeArray() {
            codes_.setUnitSize(1);
        }
        
        void setCodes(std::vector<size_t> numbers) {
            for (auto i = 0; i < numbers.size(); i++) {
                codes_.setValue(i, numbers[i]);
            }
            codes_.build();
        }
        
        size_t getCode(size_t index) const {
            return codes_.getValue(index);
        }
        
        size_t sizeInBytes() const {
            return codes_.sizeInBytes();
        }
        
    private:
        CODE_TYPE codes_;
    };
    
}

#endif /* CodeArray_hpp */

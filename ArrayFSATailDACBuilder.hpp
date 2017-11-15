//
//  ArrayFSATailDACBuilder.hpp
//
//  Created by 松本拓真 on 2017/11/14.
//

#ifndef ArrayFSATailDACBuilder_hpp
#define ArrayFSATailDACBuilder_hpp

#include "ArrayFSATailBuilder.hpp"
#include "ArrayFSATailDAC.hpp"

namespace array_fsa {
    class PlainFSA;
    
    class ArrayFSATailDACBuilder : public ArrayFSATailBuilder {
    public:
        virtual ~ArrayFSATailDACBuilder() = default;
        
        ArrayFSATailDACBuilder(const PlainFSA& orig_fsa) : ArrayFSATailBuilder(orig_fsa) {}
        
        static ArrayFSATailDAC build(const PlainFSA& orig_fsa);
        
    };
    
}

#endif /* ArrayFSATailDACBuilder_hpp */

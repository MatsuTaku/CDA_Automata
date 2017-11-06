//
//  ArrayFSATailBuilder.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#ifndef ArrayFSA_TailBuilder_hpp
#define ArrayFSA_TailBuilder_hpp

#include "ArrayFSABuilder.hpp"
#include "Rank.hpp"

namespace array_fsa {
    
    class ArrayFSATailBuilder : public ArrayFSABuilder {
    public:
        
    private:
        std::vector<uint8_t> tail_bytes_;
        struct strDictData {
            std::string string;
            size_t place;
        };
        std::vector<strDictData> str_dict_list_;
        
        void arrange_(size_t state, size_t index) override;
        
    };
    
}

#endif /* ArrayFSA_TailBuilder_hpp */

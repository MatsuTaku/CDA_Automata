//
//  ArrayFSATailBuilder.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#ifndef ArrayFSA_TailBuilder_hpp
#define ArrayFSA_TailBuilder_hpp

#include "ArrayFSABuilder.hpp"
#include "ArrayFSATail.hpp"
#include "Rank.hpp"
#include "StringDict.hpp"

namespace array_fsa {
    
    class PlainFSATail;
    
    class ArrayFSATailBuilder : public ArrayFSABuilder {
    public:
        virtual ~ArrayFSATailBuilder() = default;
        
        ArrayFSATailBuilder(const PlainFSA& orig_fsa) : ArrayFSABuilder(orig_fsa) {}
        
        static ArrayFSATail build(const PlainFSA& orig_fsa);
        
        static void showInBox(ArrayFSATailBuilder &builder, ArrayFSATail &fsa);
        
    protected:
        StringDict str_dict_;
        
        size_t get_label_number(size_t index) const {
            size_t labelIndex = 0;
            std::memcpy(&labelIndex, &bytes_[offset_(index) + 1 + kAddrSize], 4);
            return labelIndex;
        }
        
        bool has_label(size_t index) const {
            return (bytes_[offset_(index)] & 8) != 0;
        }
        
        bool is_label_start(size_t index) const {
            return str_dict_.isStartLabel(index);
        }
        
        bool is_label_finish(size_t index) const {
            return str_dict_.isEndLabel(index);
        }
        
        void set_label_number_(size_t index, size_t labelIndex) {
            std::memcpy(&bytes_[offset_(index) + 1 + kAddrSize], &labelIndex, 4);
        }
        
        void set_has_label(size_t index) {
            bytes_[offset_(index)] |= 8;
        }
        
        void build_() override;
        
        void expand_() override;
        
        void arrange_(size_t state, size_t index) override;
        
    };
    
}

#endif /* ArrayFSA_TailBuilder_hpp */

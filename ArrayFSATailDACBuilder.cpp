//
//  ArrayFSATailDACBuilder.cpp
//  bench
//
//  Created by 松本拓真 on 2017/11/14.
//

#include "ArrayFSATailDACBuilder.hpp"
#include "ArrayFSATailDAC.hpp"

using namespace array_fsa;

ArrayFSATailDAC ArrayFSATailDACBuilder::build(const PlainFSA &orig_fsa) {
    ArrayFSATailDACBuilder builder(orig_fsa);
    builder.build_();
    
    // Release
    ArrayFSATailDAC new_fsa;
    const auto num_elems = builder.num_elems_();
    
    new_fsa.calc_next_size(num_elems);
    new_fsa.element_size_ = 1 + 1;
    
    new_fsa.bytes_.resize(num_elems * new_fsa.element_size_);
    
    new_fsa.label_bytes_ = builder.str_dict_.get_label_bytes();
    new_fsa.calc_check_size(new_fsa.label_bytes_.size());
    for (auto i = 0; i < new_fsa.label_bytes_.size(); i++) {
        new_fsa.set_is_label_finish(i, builder.is_label_finish(i));
    }
    
    for (size_t i = 0; i < num_elems; ++i) {
        size_t next = i ^ builder.get_target_state_(i);
        new_fsa.set_next(i, next);
        auto hasLabel = builder.has_label(i);
        if (!hasLabel) {
            new_fsa.set_check(i, builder.get_check_(i));
        } else {
            new_fsa.set_label_index(i, builder.get_label_index(i));
        }
        new_fsa.set_is_final(i, builder.is_final_(i));
        new_fsa.set_has_label(i, hasLabel);
        
        if (builder.is_frozen_(i)) {
            ++new_fsa.num_trans_;
        }
    }
    new_fsa.buildBits();
    
//    builder.showMapping(false);
    
    //    showInBox(builder, new_fsa);
    
    return new_fsa;
}

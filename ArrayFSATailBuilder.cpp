//
//  ArrayFSATailBuilder.cpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/11/03.
//

#include "ArrayFSATailBuilder.hpp"

#include "PlainFSA.hpp"
#include "StringDictBuilder.hpp"
#include "StringDict.hpp"

using namespace array_fsa;

// MARK: - public static

ArrayFSATail ArrayFSATailBuilder::build(const PlainFSA& orig_fsa) {
    ArrayFSATailBuilder builder(orig_fsa);
    builder.build_();
    
    // Release
    ArrayFSATail new_fsa;
    const auto num_elems = builder.num_elems_();
    
    new_fsa.calc_next_size(num_elems);
    new_fsa.element_size_ = new_fsa.next_size_ + 1;
    
    new_fsa.bytes_.resize(num_elems * new_fsa.element_size_);
    
    new_fsa.label_bytes_ = builder.str_dict_.get_label_bytes();
    new_fsa.calc_check_size(new_fsa.label_bytes_.size());
    
    for (size_t i = 0; i < num_elems; ++i) {
        size_t next = i ^ builder.get_target_state_(i);
        new_fsa.set_next(i, next);
        auto hasLabel = builder.has_label(i);
        if (!hasLabel) {
            new_fsa.set_check(i, builder.get_check_(i));
        } else {
            new_fsa.set_label_index(i, builder.get_label_number(i));
        }
        new_fsa.set_is_final(i, builder.is_final_(i));
        new_fsa.set_has_label(i, hasLabel);
        
        if (builder.is_frozen_(i)) {
            ++new_fsa.num_trans_;
        }
    }
    new_fsa.buildBits();
    
    builder.showMapping(false);

//    showInBox(builder, new_fsa);
    
    return new_fsa;
}

void ArrayFSATailBuilder::showInBox(ArrayFSATailBuilder &builder, ArrayFSATail &fsa) {
    auto tab = "\t";
    for (auto i = 0; i < 256; i++) {
        auto nLabel = fsa.has_label(i) ? fsa.get_label_index(i) : fsa.get_check_(i);
        std::cout << i << tab << fsa.is_final_trans(i) << tab << fsa.has_label(i) << tab << fsa.get_next_(i) << tab << nLabel << std::endl;;
        Rank::show_as_bytes(nLabel, 4);
        std::cout << std::endl;
    }
}


// MARK: - private

void ArrayFSATailBuilder::build_() {
    
    bytes_.reserve(static_cast<size_t>(orig_fsa_.get_num_trans() * 1.1 * kElemSize));
    
    expand_();
    freeze_state_(0);
    set_true_final_and_used_next_(0);
    
    // TODO: string dict
    str_dict_ = StringDictBuilder::build(orig_fsa_);
    
    arrange_(orig_fsa_.get_root_state(), 0);
    
    // reset
    state_map_ = std::unordered_map<size_t, size_t>();
}

void ArrayFSATailBuilder::arrange_(size_t state, size_t index) {
    const auto first_trans = orig_fsa_.get_first_trans(state);
    
    if (first_trans == 0) {
        set_next_(index, index); // to the terminal state
        return;
    }
    
    { // Set next of offset to state's second if needed.
        auto it = state_map_.find(state);
        if (it != state_map_.end()) {
            // already visited state
            set_next_(index, it->second);
            return;
        }
    }
    
    const auto next = find_next_(first_trans);
    if (offset_(next) >= bytes_.size()) {
        expand_();
    }
    
    set_next_(index, next);
    state_map_.insert(std::make_pair(state, next));
    set_used_next_(next, true);
    
    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
        const auto symbol = orig_fsa_.get_trans_symbol(trans);
        const auto child_index = next ^ symbol;
        
        freeze_state_(child_index);
        
        auto transIndex = trans / PlainFSA::kTransSize;
        if (str_dict_.isLabelSource(transIndex)) {
            set_has_label(child_index);
            set_label_number_(child_index, str_dict_.startPos(transIndex));
            
            auto labelTrans = trans;
            str_dict_.traceOnLabel(labelTrans / PlainFSA::kTransSize);
            while (!str_dict_.isEndLabel() && orig_fsa_.is_straight_state(labelTrans)) {
                labelTrans = orig_fsa_.get_target_state(labelTrans);
                str_dict_.posToNext();
            }
            if (orig_fsa_.is_final_trans(labelTrans)) {
                set_final_(child_index, true);
            }
        } else {
            set_check_(child_index, symbol);
            
            if (orig_fsa_.is_final_trans(trans)) {
                set_final_(child_index, true);
            }
        }
    }
    
    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
        const auto symbol = orig_fsa_.get_trans_symbol(trans);
        
        auto labelTrans = trans;
        auto transIndex = labelTrans / PlainFSA::kTransSize;
        if (str_dict_.isLabelSource(transIndex)) {
            str_dict_.traceOnLabel(transIndex);
            while (!str_dict_.isEndLabel() && orig_fsa_.is_straight_state(labelTrans)) {
                labelTrans = orig_fsa_.get_target_state(labelTrans);
                str_dict_.posToNext();
            }
        }
        
        arrange_(orig_fsa_.get_target_state(labelTrans), next ^ symbol);
    }
}

void ArrayFSATailBuilder::expand_() {
    ArrayFSABuilder::expand_();
}


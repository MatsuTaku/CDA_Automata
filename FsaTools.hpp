//
// Created by Kampersanda on 2017/05/28.
//

#ifndef ARRAY_FSA_FSATOOLS_HPP
#define ARRAY_FSA_FSATOOLS_HPP

#include "basic.hpp"
#include "ArrayFSATail.hpp"

namespace array_fsa {
    
    struct FsaTools {
        template <typename FsaType>
        static bool is_member(const FsaType& fsa, const std::string& str) {
            size_t state = fsa.get_root_state(), arc = 0;
            
            for (char c : str) {
                arc = fsa.get_trans(state, static_cast<uint8_t>(c));
                if (arc == 0) {
                    return false;
                }
                state = fsa.get_target_state(arc);
            }
            
            return fsa.is_final_trans(arc);
        }
        
        static bool is_member(const ArrayFSATail& fsa, const std::string& str) {
            return isMemberUsingTail(fsa, str);
        }
        
        static bool is_member(const ArrayFSATailDAC& fsa, const std::string& str) {
            return isMemberUsingTail(fsa, str);
        }
        
        template <typename FSAType>
        static bool isMemberUsingTail(const FSAType &fsa, const std::string &str) {
            /*
            TransPointer pointer;
            
            for (auto i = 0; i < str.size(); i++) {
                if (!fsa.translatePointer(pointer, str[i])) {
                    return false;
                }
            }
            
            auto isFinal = fsa.is_final_trans(pointer.arc);
            auto isLabelFinish = pointer.onTheLabel ? fsa.is_label_finish(pointer.labelState) : true;
            auto isEnd = isFinal && isLabelFinish;
            return isEnd;
             */
            auto isMember = fsa.lookup(str);
            return isMember;
        }
        
    };
    
}

#endif //ARRAY_FSA_FSATOOLS_HPP

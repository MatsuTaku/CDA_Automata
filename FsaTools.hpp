//
// Created by Kampersanda on 2017/05/28.
//

#ifndef ARRAY_FSA_FSATOOLS_HPP
#define ARRAY_FSA_FSATOOLS_HPP

#include "basic.hpp"
#include "ArrayFSATail.hpp"
#include "DArrayFSA.hpp"

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
            auto isMember = fsa.isMember(str);
            return isMember;
        }
        
        static bool is_member(const double_array::DArrayFSA &fsa, const std::string &str) {
            return fsa.lookup(str);
        }
        
    };
    
}

#endif //ARRAY_FSA_FSATOOLS_HPP

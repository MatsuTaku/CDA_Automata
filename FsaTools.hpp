//
// Created by Kampersanda on 2017/05/28.
//

#ifndef ARRAY_FSA_FSATOOLS_HPP
#define ARRAY_FSA_FSATOOLS_HPP

#include "basic.hpp"
#include "ArrayFSATail.hpp"
#include "NArrayFSA.hpp"
#include "NArrayFSADACs.hpp"

namespace array_fsa {
    
    struct FsaTools {
        template <typename FSA_TYPE>
        static bool is_member(const FSA_TYPE &fsa, const std::string &str) {
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
        
        static bool is_member(const ArrayFSATail &fsa, const std::string &str) {
            return fsa.isMember(str);
        }
        
        static bool is_member(const NArrayFSA &fsa, const std::string &str) {
            return fsa.isMember(str);
        }
        
        static bool is_member(const NArrayFSADACs &fsa, const std::string &str) {
            return fsa.isMember(str);
        }
        
    };
    
}

#endif //ARRAY_FSA_FSATOOLS_HPP

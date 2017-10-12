//
// Created by Kampersanda on 2017/05/28.
//

#ifndef ARRAY_FSA_FSATOOLS_HPP
#define ARRAY_FSA_FSATOOLS_HPP

#include "basic.hpp"

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
};

}

#endif //ARRAY_FSA_FSATOOLS_HPP

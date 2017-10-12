//
// Created by Kampersanda on 2017/05/31.
//

#include <cassert>
#include <iostream>

#include "PlainFSABuilder.hpp"

using namespace array_fsa;

namespace {

std::vector<std::string> sample_strs() {
  return {
    "defied",
    "defies",
    "defy",
    "defying",
    "denied",
    "denies",
    "deny",
    "denying",
    "trie",
    "tried",
    "tries",
    "try",
    "trying"
  };
}

void test(const std::vector<std::string>& strs) {
  PlainFSA fsa;
  {
    PlainFSABuilder builder;
    for (auto& str : strs) {
      builder.add(str);
    }
    fsa = builder.release();
  }

  for (auto& str : strs) {
    auto state = fsa.get_root_state();
    size_t trans = 0;

    for (char c : str) {
      trans = fsa.get_trans(state, static_cast<uint8_t>(c));
      assert(trans != 0);
      state = fsa.get_target_state(trans);
    }
    assert(fsa.is_final_trans(trans));
  }

  fsa.print_for_debug(std::cout);
}

}

int main() {
  auto strs = sample_strs();
  test(strs);

  return 0;
}


//
// Created by Kampersanda on 2017/05/29.
//

#include <iostream>

#include "ArrayFSABuilder.hpp"
#include "MorfologikCFSA2.hpp"
#include "MorfologikFSA5.hpp"
#include "FsaTools.hpp"

using namespace array_fsa;

namespace {

template <typename FsaType>
int bench(const char* fsa_name, const char* query_name) {
  std::cout << "Bench " << FsaType::name() << " from " << fsa_name << std::endl;

  FsaType fsa;
  {
    std::ifstream ifs(fsa_name);
    if (!ifs) {
      std::cerr << "Error open " << fsa_name << std::endl;
      return 1;
    }
    fsa.read(ifs);
  }

  std::cout << "Lookup bench for " << query_name << std::endl;

  std::vector<std::string> strs;
  {
    std::ifstream ifs(query_name);
    if (!ifs) {
      std::cerr << "Error open " << query_name << std::endl;
      return 1;
    }

    for (std::string line; std::getline(ifs, line);) {
      strs.push_back(line);
    }
    strs.shrink_to_fit();
  }

  size_t ok = 0, ng = 0;
  Stopwatch sw;

  for (const auto& str : strs) {
    if (FsaTools::is_member(fsa, str)) {
      ++ok;
    } else {
      ++ng;
    }
  }

  std::cout << "Lookup time: " << sw.get_micro_sec() / (ok + ng) << " us/query" << std::endl;
  std::cout << "OK: " << ok << std::endl;
  std::cout << "NG: " << ng << std::endl;

  fsa.show_stat(std::cout);

  return 0;
}

}

int main(int argc, const char* argv[]) {
  char type = *argv[1];
  const char* fsa_name = argv[2];
  const char* query_name = argv[3];

  switch (type) {
    case '1':
      return bench<ArrayFSA>(fsa_name, query_name);
    case '2':
      return bench<MorfologikFSA5>(fsa_name, query_name);
    case '3':
      return bench<MorfologikCFSA2>(fsa_name, query_name);
    default:
      break;
  }

  return 1;
}
//
// Created by Kampersanda on 2017/05/28.
//

#ifndef ARRAY_FSA_FSATOOLS_HPP
#define ARRAY_FSA_FSATOOLS_HPP

#include "basic.hpp"
#include "Exception.hpp"

namespace array_fsa {
    
    class KeySet {
    public:
        KeySet() = delete;
        
        static std::vector<std::string> getKeySets(const char *queryName) {
            std::ifstream ifs(queryName);
            if (!ifs)
                throw DataNotFoundException(queryName);
            
            std::vector<std::string> strs;
            for (std::string line; std::getline(ifs, line);)
                strs.push_back(line);
            strs.shrink_to_fit();
            return strs;
        }
        
    };
    
    class FsaTools {
    public:
        FsaTools() = delete;
        
        template <class T>
        static T getFSAFrom(const char *fsaName) {
            std::ifstream ifs(fsaName);
            if (!ifs)
                throw DataNotFoundException(fsaName);
            T fsa;
            fsa.read(ifs);
            return fsa;
        }
        
        template <class T>
        static void measureBenchmark(const T &fsa, const char *queryName) {
            const auto strs = KeySet::getKeySets(queryName);
            auto num = strs.size();
            auto ng = 0;
            Stopwatch sw;
            for (const std::string &str : strs) {
                if (!fsa.isMember(str))
                    ++ng;
            }
            auto mSec = sw.get_micro_sec();
            
            std::cout << "Lookup time: " << mSec / num << " µs/query" << std::endl;
            std::cout << "OK: " << num - ng << std::endl;
            std::cout << "NG: " << ng << std::endl;
            
            fsa.show_stat(std::cout);
        }
        
    };
    
}

#endif //ARRAY_FSA_FSATOOLS_HPP

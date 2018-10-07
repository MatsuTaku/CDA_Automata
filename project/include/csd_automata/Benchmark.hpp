//
//  Benchmark.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/09/29.
//

#ifndef Benchmark_hpp
#define Benchmark_hpp

#include "csd_automata/basic.hpp"
#include "csd_automata/FsaTools.hpp"
#include "csd_automata/Exception.hpp"

namespace csd_automata {
    
    namespace benchmark {
        
        template <class T>
        T getFSAFrom(const char *fsaName) {
            std::ifstream ifs(fsaName);
            if (!ifs)
                throw exception::DataNotFound(fsaName);
            
            return T(ifs);
        }
        
        template <class T>
        void measureBenchmark(const T &fsa, const char *queryName, int runs, bool needsAccess) {
            const auto &strs = KeySet::getKeySets(queryName);
            auto num = strs.size();
            auto ng = 0;
            Stopwatch sw;
            for (const std::string &str : strs) {
                if (!fsa.isMember(str)) {
                    ++ng;
                    break;
                }
            }
            if (ng > 0)
                fsa.printForDebug(std::cout);
            auto mSec = sw.get_micro_sec();
            
            std::cout << "Search time: " << mSec / num << " µs/query" << std::endl;
            std::cout << "OK: " << num - ng << std::endl;
            std::cout << "NG: " << ng << std::endl;
            
            std::vector<size_t> values(strs.size());
            for (auto i = 0; i < strs.size(); i++) {
                values[i] = fsa.lookup(strs[i]);
            }
            
            sw = Stopwatch();
            for (auto r = 0; r < runs; r++) {
                ng = 0;
                for (auto i = 0; i < strs.size(); i++) {
                    if (fsa.lookup(strs[i]) != values[i])
                        ng++;
                }
            }
            mSec = sw.get_micro_sec();
            std::cout << "------" << std::endl;
            std::cout << "Lookup time on " << runs << " runs: " << mSec / runs / num << " µs/query" << std::endl;
            std::cout << "OK: " << num - ng << std::endl;
            std::cout << "NG: " << ng << std::endl;
            
            if (needsAccess) {
                sw = Stopwatch();
                for (auto r = 0; r < runs; r++) {
                    ng = 0;
                    for (auto i = 0; i < strs.size(); i++) {
                        auto &needs = strs[i];
                        const auto &getStr = fsa.access(values[i]);
                        if (getStr != needs)
                            ng++;
                    }
                }
                if (ng > 0)
                    fsa.printForDebug(std::cout);
                mSec = sw.get_micro_sec();
                std::cout << "------" << std::endl;
                std::cout << "Access time on " << runs << " runs: " << mSec / runs / num << " µs/query" << std::endl;
                std::cout << "OK: " << num - ng << std::endl;
                std::cout << "NG: " << ng << std::endl;
            }
            
            fsa.showStatus(std::cout);
        }
        
    };
    
}

#endif /* Benchmark_hpp */
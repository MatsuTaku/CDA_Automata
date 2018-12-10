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

namespace csd_automata {
    
namespace benchmark {
    
template <class T>
bool SetFSAFromFile(const char* fsa_name, T& fsa) {
    std::ifstream ifs(fsa_name);
    if (!ifs) {
        std::cout << "Not found FSA: " << fsa_name << std::endl;
        return false;
    }
    
    fsa = T(ifs);
    return true;
}

template <class T>
void MeasureBenchmark(const T& fsa, const char* queryName, int runs, bool needsAccess) {
    using std::cout, std::endl;
    
    const auto &strs = KeySet::GetKeySets(queryName);
    auto num = strs.size();
    auto ng = 0;
    Stopwatch sw;
    for (const std::string& str : strs) {
        if (!fsa.Accept(str)) {
            ++ng;
            break;
        }
    }
    if (ng > 0)
        fsa.PrintForDebug(cout);
    auto mSec = sw.get_micro_sec();
    
    cout << "Search time: " << mSec / num << " µs/query" << endl
    << "OK: " << num - ng << endl
    << "NG: " << ng << endl;
    
    std::vector<size_t> values(strs.size());
    for (auto i = 0; i < strs.size(); i++) {
        values[i] = fsa.Lookup(strs[i]);
    }
    
    sw = Stopwatch();
    for (auto r = 0; r < runs; r++) {
        ng = 0;
        for (auto i = 0; i < strs.size(); i++) {
            if (fsa.Lookup(strs[i]) != values[i])
                ng++;
        }
    }
    mSec = sw.get_micro_sec();
    cout << "------" << endl
    << "Lookup time on " << runs << " runs: " << mSec / runs / num << " µs/query" << endl
    << "OK: " << num - ng << endl
    << "NG: " << ng << endl;
    
    if (needsAccess) {
        sw = Stopwatch();
        for (auto r = 0; r < runs; r++) {
            ng = 0;
            for (auto i = 0; i < strs.size(); i++) {
                auto& needs = strs[i];
                const auto& getStr = fsa.Access(values[i]);
                if (getStr != needs)
                    ng++;
            }
        }
        if (ng > 0)
            fsa.PrintForDebug(cout);
        mSec = sw.get_micro_sec();
        cout << "------" << endl;
        cout << "Access time on " << runs << " runs: " << mSec / runs / num << " µs/query" << endl;
        cout << "OK: " << num - ng << endl;
        cout << "NG: " << ng << endl;
    }
    
    fsa.ShowStats(cout);
}
    
}
    
}

#endif /* Benchmark_hpp */

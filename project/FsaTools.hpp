//
// Created by Kampersanda on 2017/05/28.
//

#ifndef ARRAY_FSA_FSATOOLS_HPP
#define ARRAY_FSA_FSATOOLS_HPP

#include "array_fsa.hpp"
#include "csd_automata/Exception.hpp"
#include "MarisaWrapper.hpp"

namespace csd_automata {
    
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
        
        static void setMarisaKeyset(const char *queryName, marisa::Keyset* keyset) {
            std::ifstream ifs(queryName);
            if (!ifs)
                throw DataNotFoundException(queryName);
            
            for (std::string line; std::getline(ifs, line);) {
                std::string::size_type delim_pos = line.find_last_of('\t');
                float weight = 1.0F;
                if (delim_pos != line.npos) {
                    char *end_of_value;
                    weight = (float)std::strtod(&line[delim_pos + 1], &end_of_value);
                    if (*end_of_value == '\0') {
                        line.resize(delim_pos);
                    }
                }
                keyset->push_back(line.c_str(), line.length());
            }
        }
        
    };
    
    namespace bench {
        
        template <class T>
        static T getFSAFrom(const char *fsaName);
        
        template <class T>
        static void measureBenchmark(const T &fsa, const char *queryName, int runs, bool needsAccess);
        
    };
    
    template <class T>
    T bench::getFSAFrom(const char *fsaName) {
        std::ifstream ifs(fsaName);
        if (!ifs)
            throw DataNotFoundException(fsaName);
        
        return T(ifs);
    }
    
    template <>
    MarisaWrapper bench::getFSAFrom(const char *fsaName) {
        return MarisaWrapper(fsaName);
    }
    
    template <class T>
    void bench::measureBenchmark(const T &fsa, const char *queryName, int runs, bool needsAccess) {
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
    
    template <>
    void bench::measureBenchmark(const MarisaWrapper &fsa, const char *queryName, int runs, bool needsAccess) {
        marisa::Keyset keyset;
        KeySet::setMarisaKeyset(queryName, &keyset);
        auto num = keyset.size();
        marisa::Agent agent;
        auto ng = 0;
        std::vector<size_t> values(num);
        for (auto i = 0; i < num; i++) {
            agent.set_query(keyset[i].ptr(), keyset[i].length());
            if (!fsa.isMember(agent))
                ++ng;
            values[i] = agent.key().id();
        }
        
        Stopwatch sw;
        for (auto r = 0; r < runs; r++) {
            ng = 0;
            for (auto i = 0; i < keyset.size(); i++) {
                agent.set_query(keyset[i].ptr(), keyset[i].length());
                if (!fsa.lookup(agent) || agent.key().id() != values[i])
                    ng++;
            }
        }
        auto mSec = sw.get_micro_sec();
        std::cout << "------" << std::endl;
        std::cout << "Lookup time on " << runs << " runs: " << mSec / runs / num << " µs/query" << std::endl;
        std::cout << "OK: " << num - ng << std::endl;
        std::cout << "NG: " << ng << std::endl;
        
        if (needsAccess) {
            sw = Stopwatch();
            for (auto r = 0; r < runs; r++) {
                ng = 0;
                for (auto i = 0; i < keyset.size(); i++) {
                    agent.set_query(values[i]);
                    fsa.access(agent);
                    if (agent.key().length() != keyset[i].length() ||
                        std::memcmp(agent.key().ptr(), keyset[i].ptr(), agent.key().length()) != 0)
                        ng++;
                }
            }
            mSec = sw.get_micro_sec();
            std::cout << "------" << std::endl;
            std::cout << "Access time on " << runs << " runs: " << mSec / runs / num << " µs/query" << std::endl;
            std::cout << "OK: " << num - ng << std::endl;
            std::cout << "NG: " << ng << std::endl;
        }
        
        fsa.showStatus(std::cout);
    }
    
}

#endif //ARRAY_FSA_FSATOOLS_HPP

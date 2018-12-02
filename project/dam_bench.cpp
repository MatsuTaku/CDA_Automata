//
// Created by Kampersanda on 2017/05/29.
//

#include "csd_automata.hpp"
#include "csd_automata/Benchmark.hpp"
#include "XcdatWrapper.hpp"
#include "MarisaWrapper.hpp"
#include "DartsCloneWrapper.hpp"

using namespace csd_automata;

namespace {
    
#ifdef NDEBUG
    const int RUNS = 10;
#else
    const int RUNS = 1;
#endif
    
    void measureBenchmark(const wrapper::MarisaWrapper& marisa, const char* query_name, bool needs_access) {
        marisa::Keyset keyset;
        wrapper::SetMarisaKeySet(query_name, &keyset);
        auto num = keyset.size();
        marisa::Agent agent;
        auto ng = 0;
        std::vector<size_t> values(num);
        for (auto i = 0; i < num; i++) {
            agent.set_query(keyset[i].ptr(), keyset[i].length());
            if (!marisa.Accept(agent))
                ++ng;
            values[i] = agent.key().id();
        }
        
        csd_automata::Stopwatch sw;
        for (auto r = 0; r < RUNS; r++) {
            ng = 0;
            for (auto i = 0; i < keyset.size(); i++) {
                agent.set_query(keyset[i].ptr(), keyset[i].length());
                if (!marisa.Lookup(agent) || agent.key().id() != values[i])
                    ng++;
            }
        }
        auto mSec = sw.get_micro_sec();
        std::cout << "------" << std::endl;
        std::cout << "Lookup time on " << RUNS << " runs: " << mSec / RUNS / num << " µs/query" << std::endl;
        std::cout << "OK: " << num - ng << std::endl;
        std::cout << "NG: " << ng << std::endl;
        
        if (needs_access) {
            sw = csd_automata::Stopwatch();
            for (auto r = 0; r < RUNS; r++) {
                ng = 0;
                for (auto i = 0; i < keyset.size(); i++) {
                    agent.set_query(values[i]);
                    marisa.Access(agent);
                    if (agent.key().length() != keyset[i].length() ||
                        std::memcmp(agent.key().ptr(), keyset[i].ptr(), agent.key().length()) != 0)
                        ng++;
                }
            }
            mSec = sw.get_micro_sec();
            std::cout << "------" << std::endl;
            std::cout << "Access time on " << RUNS << " runs: " << mSec / RUNS / num << " µs/query" << std::endl;
            std::cout << "OK: " << num - ng << std::endl;
            std::cout << "NG: " << ng << std::endl;
        }
        
        marisa.ShowStatus(std::cout);
    }
    
    void measureBenchmark(const wrapper::DartsCloneWrapper& da, const char* queryName, bool needsAccess) {
        std::vector<char*> strs;
        size_t num;
        {
            const auto &sets = csd_automata::KeySet::GetKeySets(queryName);
            num = sets.size();
            for (auto &key : sets) {
                strs.push_back(new char[key.size() + 1]);
                strcpy(strs.back(), key.c_str());
                strs.back()[key.size()]  = '\0';
            }
        }
        
        auto ng = 0;
        csd_automata::Stopwatch sw;
        
        std::vector<size_t> values(num);
        for (auto i = 0; i < num; i++) {
            values[i] = da.Lookup(strs[i]);
        }
        
        sw = csd_automata::Stopwatch();
        for (auto r = 0; r < RUNS; r++) {
            ng = 0;
            for (auto i = 0; i < num; i++) {
                if (da.Lookup(strs[i]) != values[i])
                    ng++;
            }
        }
        auto mSec = sw.get_micro_sec();
        std::cout << "------" << std::endl;
        std::cout << "Lookup time on " << RUNS << " runs: " << mSec / RUNS / num << " µs/query" << std::endl;
        std::cout << "OK: " << num - ng << std::endl;
        std::cout << "NG: " << ng << std::endl;
        
        // Not support access!
        
        da.ShowStatus(std::cout);
        
        for (auto &str : strs)
            delete str;
    }
    
    int benchMarisa(const char* fsaName, const char* queryName, bool needsAccess = true) {
        try {
            std::cout << "Bench " << wrapper::MarisaWrapper::name() << " from " << fsaName << std::endl;
            
            wrapper::MarisaWrapper fsa(fsaName);
            
            std::cout << "Search benchmark for " << queryName << std::endl;
            
            measureBenchmark(fsa, queryName, needsAccess);
            
        } catch (exception::DataNotFound& e) {
            std::cerr << e.what() << std::endl;
            return -1;
        } catch (exception::DoesntHaveMember& e) {
            std::cout << e.what() << std::endl;
            return -1;
        }
        return 0;
    }
    
    int benchDarts(const char* fsaName, const char* queryName, bool needsAccess = true) {
        try {
            std::cout << "Bench " << wrapper::DartsCloneWrapper::name() << " from " << fsaName << std::endl;
            
            wrapper::DartsCloneWrapper darts(fsaName);
            
            std::cout << "Search benchmark for " << queryName << std::endl;
            
            measureBenchmark(darts, queryName, needsAccess);
            
        } catch (exception::DataNotFound e) {
            std::cerr << e.what() << std::endl;
            return -1;
        } catch (exception::DoesntHaveMember e) {
            std::cout << e.what() << std::endl;
            return -1;
        }
        return 0;
    }
    
    template <typename T>
    int bench(const char* fsa_name, const char* query_name, bool needsAccess = true) {
        try {
            std::cout << "Bench " << T::name() << " from " << fsa_name << std::endl;
            
            T fsa = benchmark::GetFSAFrom<T>(fsa_name);
            
            std::cout << "Search benchmark for " << query_name << std::endl;
            
            benchmark::MeasureBenchmark(fsa, query_name, RUNS, needsAccess);
            
        } catch (exception::DataNotFound e) {
            std::cerr << e.what() << std::endl;
            return -1;
        } catch (exception::DoesntHaveMember e) {
            std::cout << e.what() << std::endl;
            return -1;
        }
        return 0;
    }
    
}

int main(int argc, const char* argv[]) {
    auto fsa_name = argv[1];
    auto query_name = argv[2];
    auto type = atoi(argv[3]);
    
#ifndef NDEBUG
    fsa_name = "../../results/jawiki-20181001/jawiki-20181001.dam";
    query_name = "../../data-sets/local/jawiki-20181001.dict";
    type = 11;
#endif
    
    switch (type) {
        case 0:
            return bench<DaFsa>(fsa_name, query_name);
        case 1:
            return bench<DaFsaDac>(fsa_name, query_name);
        case 11:
            return bench<SdLoDaFsa>(fsa_name, query_name, false);
        case 2:
            return bench<SdDaFsa>(fsa_name, query_name);
        case 3:
            return bench<DoubleArrayCFSA<false, true, true, true, true>>(fsa_name, query_name);
        case 4:
            return bench<DoubleArrayCFSA<true, false, true, true, true>>(fsa_name, query_name);
        case 5:
            return bench<DoubleArrayCFSA<true, true, false, true, true>>(fsa_name, query_name);
        case 6:
            return bench<DoubleArrayCFSA<true, true, true, false, true>>(fsa_name, query_name);
        case 7:
            return bench<wrapper::XcdatWrapper<false>>(fsa_name, query_name);
        case 8:
            return benchMarisa(fsa_name, query_name);
        case 9:
            return bench<SdMrfFsa5>(fsa_name, query_name);
        case 10:
            return bench<SdMrfCfsa2>(fsa_name, query_name);
        case 12:
            return benchDarts(fsa_name, query_name);
        default:
            std::cerr << "type is invalid value: " << type << std::endl;
            return -1;
    }

}

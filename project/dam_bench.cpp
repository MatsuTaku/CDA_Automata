//
// Created by Kampersanda on 2017/05/29.
//

#include "csd_automata.hpp"
#include "csd_automata/Benchmark.hpp"
#include "XcdatWrapper.hpp"
#include "MarisaWrapper.hpp"
#include "DartsCloneWrapper.hpp"

namespace {
    
#ifdef NDEBUG
const int RUNS = 10;
#else
const int RUNS = 1;
#endif

template <typename T>
void bench(const char* fsa_name, const char* query_name, bool needsAccess = true) {
    std::cout << "Bench " << T::name() << " from " << fsa_name << std::endl;
    auto ifs = csd_automata::GetStreamOrDie<std::ifstream>(fsa_name);
    T fsa(ifs);
    
    std::cout << "Search benchmark for " << query_name << std::endl;
    csd_automata::benchmark::MeasureBenchmark(fsa, query_name, RUNS, needsAccess);
}
    
void benchMarisa(const char* fsa_name, const char* query_name, bool needs_access = true) {
    std::cout << "Bench " << wrapper::MarisaWrapper::name() << " from " << fsa_name << std::endl;
    wrapper::MarisaWrapper marisa(fsa_name);
    
    std::cout << "Search benchmark for " << query_name << std::endl;
    marisa::Keyset keyset;
    auto ifs = csd_automata::GetStreamOrDie<std::ifstream>(query_name);
    wrapper::SetMarisaKeySet(ifs, &keyset);
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

void benchDarts(const char* fsa_name, const char* query_name) {
    std::cout << "Bench " << wrapper::DartsCloneWrapper::name() << " from " << fsa_name << std::endl;
    wrapper::DartsCloneWrapper darts(fsa_name);
    
    std::cout << "Search benchmark for " << query_name << std::endl;
    //    measureBenchmark(darts, queryName, needsAccess);
    std::vector<char*> strs;
    size_t num;
    {
        const auto &sets = csd_automata::KeySet::GetKeySets(query_name);
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
        values[i] = darts.Lookup(strs[i]);
    }
    
    sw = csd_automata::Stopwatch();
    for (auto r = 0; r < RUNS; r++) {
        ng = 0;
        for (auto i = 0; i < num; i++) {
            if (darts.Lookup(strs[i]) != values[i])
                ng++;
        }
    }
    auto mSec = sw.get_micro_sec();
    std::cout << "------" << std::endl;
    std::cout << "Lookup time on " << RUNS << " runs: " << mSec / RUNS / num << " µs/query" << std::endl;
    std::cout << "OK: " << num - ng << std::endl;
    std::cout << "NG: " << ng << std::endl;
    
    // Not support access!
    
    darts.ShowStatus(std::cout);
    
    for (auto &str : strs)
        delete str;
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
            bench<csd_automata::DaFsa>(fsa_name, query_name);
            break;
        case 1:
            bench<csd_automata::DaFsaDac>(fsa_name, query_name);
            break;
        case 11:
            bench<csd_automata::SdLoDaFsa>(fsa_name, query_name, false);
            break;
        case 2:
            bench<csd_automata::SdDaFsa>(fsa_name, query_name);
            break;
        case 3:
            bench<csd_automata::DoubleArrayCFSA<false, true, true, true, true>>(fsa_name, query_name);
            break;
        case 4:
            bench<csd_automata::DoubleArrayCFSA<true, false, true, true, true>>(fsa_name, query_name);
            break;
        case 5:
            bench<csd_automata::DoubleArrayCFSA<true, true, false, true, true>>(fsa_name, query_name);
            break;
        case 6:
            bench<csd_automata::DoubleArrayCFSA<true, true, true, false, true>>(fsa_name, query_name);
            break;
        case 7:
            bench<wrapper::XcdatWrapper<false>>(fsa_name, query_name);
            break;
        case 13:
            bench<wrapper::XcdatWrapper<true>>(fsa_name, query_name);
            break;
        case 8:
            benchMarisa(fsa_name, query_name);
            break;
        case 9:
            bench<csd_automata::SdMrfFsa5>(fsa_name, query_name);
            break;
        case 10:
            bench<csd_automata::SdMrfCfsa2>(fsa_name, query_name);
            break;
        case 12:
            benchDarts(fsa_name, query_name);
            break;
        default:
            std::cerr << "type is invalid value: " << type << std::endl;
            return -1;
    }
    
    return 0;
}

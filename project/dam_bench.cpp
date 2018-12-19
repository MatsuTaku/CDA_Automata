//
// Created by Kampersanda on 2017/05/29.
//

#include "csd_automata.hpp"
#include "csd_automata/basic.hpp"
#include "csd_automata/FsaTools.hpp"
#include "csd_automata/FsaTools.hpp"
#include "XcdatWrapper.hpp"
#include "MarisaWrapper.hpp"
#include "DartsCloneWrapper.hpp"

namespace {
    
#ifdef NDEBUG
constexpr int kRuns = 10;
#else
constexpr int kRuns = 1;
#endif

template <typename Fsa>
void BenchFsa(const char* fsa_name, const char* query_name, bool needs_access = true) {
    std::cout << "Bench " << Fsa::name() << " from " << fsa_name << std::endl;
    auto ifs = csd_automata::GetStreamOrDie<std::ifstream>(fsa_name);
    Fsa fsa(ifs);
    
    std::cout << "Search benchmark for " << query_name << std::endl;
    using std::cout, std::endl;
    
    const auto& strs = csd_automata::GetKeySets(query_name);
    auto num = strs.size();
    auto ng = 0;
    csd_automata::Stopwatch sw;
    for (auto& str : strs) {
        if (!fsa.Accept(str)) {
            ++ng;
            break;
        }
    }
    if (ng > 0)
        fsa.PrintForDebug(cout);
    auto mic_sec = sw.get_micro_sec();
    
    cout << "Search time: " << mic_sec / num << " µs/query" << endl
    << "OK: " << num - ng << endl
    << "NG: " << ng << endl;
    
    std::vector<size_t> values(strs.size());
    for (auto i = 0; i < strs.size(); i++) {
        values[i] = fsa.Lookup(strs[i]);
    }
    
    sw = csd_automata::Stopwatch();
    for (auto r = 0; r < kRuns; r++) {
        ng = 0;
        for (auto i = 0; i < strs.size(); i++) {
            if (fsa.Lookup(strs[i]) != values[i])
                ng++;
        }
    }
    mic_sec = sw.get_micro_sec();
    cout << "------" << endl
    << "Lookup time on " << kRuns << " kRuns: " << mic_sec / kRuns / num << " µs/query" << endl
    << "OK: " << num - ng << endl
    << "NG: " << ng << endl;
    
    if (needs_access) {
        sw = csd_automata::Stopwatch();
        for (auto r = 0; r < kRuns; r++) {
            ng = 0;
            for (auto i = 0; i < strs.size(); i++) {
                auto& needs = strs[i];
                const auto& get_str = fsa.Access(values[i]);
                if (get_str != needs)
                    ng++;
            }
        }
        if (ng > 0)
            fsa.PrintForDebug(cout);
        mic_sec = sw.get_micro_sec();
        cout << "------" << endl;
        cout << "Access time on " << kRuns << " kRuns: " << mic_sec / kRuns / num << " µs/query" << endl;
        cout << "OK: " << num - ng << endl;
        cout << "NG: " << ng << endl;
    }
    
    cout << endl;
    fsa.ShowStats(cout);
}
    
void BenchMarisa(const char* fsa_name, const char* query_name, bool needs_access = true) {
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
    for (auto r = 0; r < kRuns; r++) {
        ng = 0;
        for (auto i = 0; i < keyset.size(); i++) {
            agent.set_query(keyset[i].ptr(), keyset[i].length());
            if (!marisa.Lookup(agent) || agent.key().id() != values[i])
                ng++;
        }
    }
    auto mic_sec = sw.get_micro_sec();
    std::cout << "------" << std::endl;
    std::cout << "Lookup time on " << kRuns << " kRuns: " << mic_sec / kRuns / num << " µs/query" << std::endl;
    std::cout << "OK: " << num - ng << std::endl;
    std::cout << "NG: " << ng << std::endl;
    
    if (needs_access) {
        sw = csd_automata::Stopwatch();
        for (auto r = 0; r < kRuns; r++) {
            ng = 0;
            for (auto i = 0; i < keyset.size(); i++) {
                agent.set_query(values[i]);
                marisa.Access(agent);
                if (agent.key().length() != keyset[i].length() ||
                    std::memcmp(agent.key().ptr(), keyset[i].ptr(), agent.key().length()) != 0)
                    ng++;
            }
        }
        mic_sec = sw.get_micro_sec();
        std::cout << "------" << std::endl;
        std::cout << "Access time on " << kRuns << " kRuns: " << mic_sec / kRuns / num << " µs/query" << std::endl;
        std::cout << "OK: " << num - ng << std::endl;
        std::cout << "NG: " << ng << std::endl;
    }
    
    marisa.ShowStatus(std::cout);
}

void BenchDarts(const char* fsa_name, const char* query_name) {
    std::cout << "Bench " << wrapper::DartsCloneWrapper::name() << " from " << fsa_name << std::endl;
    wrapper::DartsCloneWrapper darts(fsa_name);
    
    std::cout << "Search benchmark for " << query_name << std::endl;
    std::vector<char*> strs;
    size_t num;
    {
        const auto& sets = csd_automata::GetKeySets(query_name);
        num = sets.size();
        for (auto& key : sets) {
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
    for (auto r = 0; r < kRuns; r++) {
        ng = 0;
        for (auto i = 0; i < num; i++) {
            if (darts.Lookup(strs[i]) != values[i])
                ng++;
        }
    }
    auto mic_sec = sw.get_micro_sec();
    std::cout << "------" << std::endl;
    std::cout << "Lookup time on " << kRuns << " kRuns: " << mic_sec / kRuns / num << " µs/query" << std::endl;
    std::cout << "OK: " << num - ng << std::endl;
    std::cout << "NG: " << ng << std::endl;
    
    // Not support access!
    
    darts.ShowStatus(std::cout);
    
    for (auto& str : strs)
        delete str;
}
    
}


int main(int argc, const char* argv[]) {
    auto fsa_name = argv[1];
    auto query_name = argv[2];
    auto type = atoi(argv[3]);
    
#ifndef NDEBUG
    fsa_name = "../../results/jawiki-20181001/jawiki-20181001.dam";
//    fsa_name = "../../results/jawiki-20181001/jawiki-20181001.morfologik_fsa5";
//    fsa_name = "../../results/jawiki-20181001/jawiki-20181001.morfologik_cfsa2d";
    query_name = "../../data-sets/local/jawiki-20181001.dict";
//    fsa_name = "../../results/wikipedia2/wikipedia2.dam";
//    query_name = "../../data-sets/weiss/wikipedia2.dict";
//    fsa_name = "../../results/abc/abc.dam";
//    query_name = "../../data-sets/ciura-deorowicz/abc.dict";
//    query_name = "../../data-sets/ciura-deorowicz/abc.1000000.rnd_dict";
    type = 11;
#endif
    
    switch (type) {
//        case 0:
//            BenchFsa<csd_automata::DaFsa>(fsa_name, query_name);
//            break;
//        case 1:
//            BenchFsa<csd_automata::DaFsaDac>(fsa_name, query_name);
//            break;
        case 11:
            BenchFsa<csd_automata::SdLoDaFsa>(fsa_name, query_name, false);
            break;
        case 15:
            BenchFsa<csd_automata::SdLoCidDaFsa>(fsa_name, query_name, false);
            break;
        case 16:
            BenchFsa<csd_automata::SdLoDacDaFsa>(fsa_name, query_name, false);
            break;
        case 14:
            BenchFsa<csd_automata::SdLoSiDaFsa>(fsa_name, query_name, false);
            break;
        case 2:
            BenchFsa<csd_automata::SdDaFsa>(fsa_name, query_name);
            break;
        case 7:
            BenchFsa<wrapper::XcdatWrapper<false>>(fsa_name, query_name);
            break;
        case 13:
            BenchFsa<wrapper::XcdatWrapper<true>>(fsa_name, query_name);
            break;
        case 9:
            BenchFsa<csd_automata::SdMrfFsa5>(fsa_name, query_name);
            break;
        case 10:
            BenchFsa<csd_automata::SdMrfCFsa2>(fsa_name, query_name);
            break;
        case 8:
            BenchMarisa(fsa_name, query_name);
            break;
        case 12:
            BenchDarts(fsa_name, query_name);
            break;
        default:
            std::cerr << "type is invalid value: " << type << std::endl;
            return -1;
    }
    
    return 0;
}

//
// Created by Kampersanda on 2017/05/29.
//

#include "csd_automata/MorfologikFsaDictionary.hpp"
#include "XcdatWrapper.hpp"
#include "MarisaWrapper.hpp"
#include "DartsCloneWrapper.hpp"
#include "CentroidWrapper.hpp"

#include "csd_automata/util.hpp"

namespace wrapper {

void LoadFromFile(csd_automata::SdMrfFsa5& self, std::string file_name) {
    csd_automata::LoadFromFile(self, file_name);
}

void LoadFromFile(csd_automata::SdMrfCFsa2& self, std::string file_name) {
    csd_automata::LoadFromFile(self, file_name);
}

}

namespace {

#ifdef NDEBUG
constexpr int kRuns = 10;
#else
constexpr int kRuns = 1;
#endif

template <class Dictionary, bool DoExtraction>
class Benchmarker {
public:
    static void benchmark(const std::string fsa_name, const std::string query_name, const std::string results_name) {
        using std::cout, std::endl;
        cout << "Bench " << Dictionary::name() << " from " << fsa_name << endl;
        Dictionary dict;
        wrapper::LoadFromFile(dict, fsa_name);
        
        cout << "Search benchmark for " << query_name << endl;
        
        const auto& queries = csd_automata::util::GetKeySets(query_name);
        auto num = queries.size();
        auto ng = 0;
        // warm up
        auto mic_sec = csd_automata::util::MeasureProcessingMicro([&] {
            for (const auto& str : queries) {
                if (!dict.Accept(str)) {
                    ++ng;
                }
            }
        });
        if (ng > 0) {
            dict.PrintForDebug(cout);
            return;
        }
        
        cout << "Search time: " << mic_sec / num << " µs/query" << endl
        << "OK: " << num - ng << endl
        << "NG: " << ng << endl;
        
        auto lookup_time = csd_automata::util::MeasureProcessingMicro([&] {
            for (auto r = 0; r < kRuns; r++) {
                auto id = 0; // never used
                for (auto i = 0; i < queries.size(); i++) {
                    id = dict.Lookup(queries[i]);
                }
            }
        });
        cout << "------" << endl
        << "Lookup time on " << kRuns << " kRuns: " << lookup_time / kRuns / num << " µs/query" << endl;
        
        std::vector<double> times = {lookup_time};
        
        if (DoExtraction) {
            auto access_time = csd_automata::util::MeasureProcessingMicro([&] {
                for (auto r = 0; r < kRuns; r++) {
                    std::string extracted = ""; // never used
                    for (size_t i = 0; i < queries.size(); i++) {
                        extracted = dict.Access(i + 1);
                    }
                }
            });
            cout << "------" << endl
            << "Access time on " << kRuns << " kRuns: " << access_time / kRuns / num << " µs/query" << endl;
            times.push_back(access_time);
        }
        
        cout << endl;
        dict.ShowStats(cout);
        
        if (results_name != "") {
            std::ofstream results(results_name, std::ios::app);
            auto delim = ", ";
            results << Dictionary::tag() << delim
            << dict.size_in_bytes() << delim
            << times[0] / kRuns / num;
            if constexpr (DoExtraction)
                results << delim << times[1] / kRuns / num;
            results << endl;
        }
    }
};

template<bool DoExtraction>
class Benchmarker<wrapper::MarisaWrapper, DoExtraction> {
public:
    static void benchmark(const std::string fsa_name, const std::string query_name, const std::string results_name) {
        std::cout << "Bench " << wrapper::MarisaWrapper::name() << " from " << fsa_name << std::endl;
        wrapper::MarisaWrapper marisa;
        wrapper::LoadFromFile(marisa, fsa_name.c_str());
        
        std::cout << "Search benchmark for " << query_name << std::endl;
        marisa::Keyset keyset;
        auto ifs = csd_automata::util::GetStreamOrDie<std::ifstream>(query_name);
        wrapper::SetMarisaKeySet(ifs, &keyset);
        auto num = keyset.size();
        marisa::Agent agent;
        auto ng = 0;
        for (auto i = 0; i < num; i++) {
            agent.set_query(keyset[i].ptr(), keyset[i].length());
            if (!marisa.Accept(agent))
                ++ng;
        }
        if (ng > 0) {
            std::cout << "Some of queries are not stored!" << std::endl;
            return;
        }
        
        auto lookup_time = csd_automata::util::MeasureProcessingMicro([&] {
            for (auto r = 0; r < kRuns; r++) {
                size_t id = 0;
                for (auto i = 0; i < keyset.size(); i++) {
                    agent.set_query(keyset[i].ptr(), keyset[i].length());
                    id = marisa.Lookup(agent);
                }
            }
        });
        std::cout << "------" << std::endl
        << "Lookup time on " << kRuns << " kRuns: " << lookup_time / kRuns / num << " µs/query" << std::endl;
        
        std::vector<double> times = {lookup_time};
        
        if constexpr (DoExtraction) {
            auto access_time = csd_automata::util::MeasureProcessingMicro([&] {
                for (auto r = 0; r < kRuns; r++) {
                    for (auto i = 0; i < keyset.size(); i++) {
                        marisa.Access(agent);
                    }
                }
            });
            std::cout << "------" << std::endl
            << "Access time on " << kRuns << " kRuns: " << access_time / kRuns / num << " µs/query" << std::endl;
            times.push_back(access_time);
        }
        
        marisa.ShowStatus(std::cout);
        
        if (results_name != "") {
            std::ofstream results(results_name, std::ios::app);
            auto delim = ", ";
            results << wrapper::MarisaWrapper::tag() << delim
            << marisa.size_in_bytes() << delim
            << times[0] / kRuns / num;
            if constexpr (DoExtraction)
                results << delim << times[1] / num / kRuns;
            results << std::endl;
        }
    }
};

template<>
class Benchmarker<wrapper::DartsCloneWrapper, false> {
public:
    static void benchmark(const std::string fsa_name, const std::string query_name, const std::string results_name) {
        std::cout << "Bench " << wrapper::DartsCloneWrapper::name() << " from " << fsa_name << std::endl;
        wrapper::DartsCloneWrapper darts;
        wrapper::LoadFromFile(darts, fsa_name.c_str());
        
        std::cout << "Search benchmark for " << query_name << std::endl;
        std::vector<char*> strs;
        size_t num;
        auto sets = csd_automata::util::GetKeySets(query_name);
        num = sets.size();
        for (auto& key : sets) {
            strs.push_back(new char[key.size() + 1]);
            strcpy(strs.back(), key.c_str());
            strs.back()[key.size()]  = '\0';
        }
        
        std::vector<size_t> values(num);
        for (auto i = 0; i < num; i++) {
            values[i] = darts.Lookup(strs[i]);
        }
        
        auto lookup_time = csd_automata::util::MeasureProcessingMicro([&] {
            for (auto r = 0; r < kRuns; r++) {
                size_t id = 0; // never used
                for (auto i = 0; i < num; i++) {
                    id = darts.Lookup(strs[i]);
                }
            }
        });
        std::cout << "------" << std::endl;
        std::cout << "Lookup time on " << kRuns << " kRuns: " << lookup_time / kRuns / num << " µs/query" << std::endl;
        
        // Not support access!
        
        darts.ShowStatus(std::cout);
        
        if (results_name != "") {
            std::ofstream results(results_name, std::ios::app);
            auto delim = ", ";
            results << wrapper::DartsCloneWrapper::tag() << delim
            << darts.size_in_bytes() << delim
            << lookup_time / kRuns / num << std::endl;
        }
        
        for (auto* str : strs)
            delete str;
    }
};

template <bool DoExtraction>
class Benchmarker<wrapper::CentroidWrapper, DoExtraction> {
public:
    static void benchmark(const std::string fsa_name, const std::string query_name, const std::string results_name) {
        using std::cout, std::endl;
        cout << "Bench " << wrapper::CentroidWrapper::name() << " from " << fsa_name << endl;
        wrapper::CentroidWrapper dict;
        boost::iostreams::mapped_file_source m(fsa_name);
        succinct::mapper::map(dict, m, succinct::mapper::map_flags::warmup);
        
        cout << "Search benchmark for " << query_name << endl;
        
        const auto& queries = csd_automata::util::GetKeySets(query_name);
        auto num = queries.size();
        auto ng = 0;
        // warm up
        auto mic_sec = csd_automata::util::MeasureProcessingMicro([&] {
            for (const auto& str : queries) {
                if (!dict.Accept(str)) {
                    ++ng;
                }
            }
        });
        if (ng > 0) {
            dict.PrintForDebug(cout);
            return;
        }
        
        cout << "Search time: " << mic_sec / num << " µs/query" << endl
        << "OK: " << num - ng << endl
        << "NG: " << ng << endl;
        
        auto lookup_time = csd_automata::util::MeasureProcessingMicro([&] {
            for (auto r = 0; r < kRuns; r++) {
                auto id = 0; // never used
                for (auto i = 0; i < queries.size(); i++) {
                    id = dict.Lookup(queries[i]);
                }
            }
        });
        cout << "------" << endl
        << "Lookup time on " << kRuns << " kRuns: " << lookup_time / kRuns / num << " µs/query" << endl;
        
        std::vector<double> times = {lookup_time};
        
        if (DoExtraction) {
            auto access_time = csd_automata::util::MeasureProcessingMicro([&] {
                for (auto r = 0; r < kRuns; r++) {
                    std::string extracted = ""; // never used
                    for (size_t i = 0; i < queries.size(); i++) {
                        extracted = dict.Access(i);
                    }
                }
            });
            cout << "------" << endl
            << "Access time on " << kRuns << " kRuns: " << access_time / kRuns / num << " µs/query" << endl;
            times.push_back(access_time);
        }
        
        cout << endl;
        dict.ShowStats(cout);
        
        if (results_name != "") {
            std::ofstream results(results_name, std::ios::app);
            auto delim = ", ";
            results << wrapper::CentroidWrapper::tag() << delim
            << dict.size_in_bytes() << delim
            << times[0] / kRuns / num;
            if constexpr (DoExtraction)
                results << delim << times[1] / kRuns / num;
            results << endl;
        }
    }
};

}


int main(int argc, const char* argv[]) {
    auto dict_name = argv[1];
    auto query_name = argv[2];
    auto type = atoi(argv[3]);
    std::string results_name = argv[4];
    
#ifndef NDEBUG
    dict_name = "../../results/jawiki-20181001/jawiki-20181001.centrp";
    query_name = "../../data-sets/local/jawiki-20181001.dict";
    type = 7;
    results_name = "../../results/jawiki-20181001/jawiki-20181001.cent_results";
#endif
    
    switch (type) {
        case 1:
            Benchmarker<csd_automata::SdMrfFsa5, true>::benchmark(dict_name, query_name, results_name);
            break;
        case 2:
            Benchmarker<csd_automata::SdMrfCFsa2, true>::benchmark(dict_name, query_name, results_name);
            break;
        case 3:
            Benchmarker<wrapper::XcdatWrapper<false>, true>::benchmark(dict_name, query_name, results_name);
            break;
        case 4:
            Benchmarker<wrapper::XcdatWrapper<true>, true>::benchmark(dict_name, query_name, results_name);
            break;
        case 5:
            Benchmarker<wrapper::MarisaWrapper, true>::benchmark(dict_name, query_name, results_name);
            break;
        case 6:
            Benchmarker<wrapper::DartsCloneWrapper, false>::benchmark(dict_name, query_name, results_name);
            break;
        case 7:
            Benchmarker<wrapper::CentroidWrapper, true>::benchmark(dict_name, query_name, results_name);
            break;
        default:
            std::cerr << "type is invalid value: " << type << std::endl;
            return -1;
    }
    
    return 0;
}

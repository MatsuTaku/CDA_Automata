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
    
    void measureBenchmark(const wrapper::MarisaWrapper &fsa, const char *queryName, int runs, bool needsAccess) {
        marisa::Keyset keyset;
        wrapper::setMarisaKeyset(queryName, &keyset);
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
        
        csd_automata::Stopwatch sw;
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
            sw = csd_automata::Stopwatch();
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
    
    void measureBenchmark(const wrapper::DartsCloneWrapper &da, const char *queryName, int runs, bool needsAccess) {
        std::vector<char *> strs;
        size_t num;
        {
            const auto &sets = csd_automata::KeySet::getKeySets(queryName);
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
            values[i] = da.lookup(strs[i]);
        }
        
        sw = csd_automata::Stopwatch();
        for (auto r = 0; r < runs; r++) {
            ng = 0;
            for (auto i = 0; i < num; i++) {
                if (da.lookup(strs[i]) != values[i])
                    ng++;
            }
        }
        auto mSec = sw.get_micro_sec();
        std::cout << "------" << std::endl;
        std::cout << "Lookup time on " << runs << " runs: " << mSec / runs / num << " µs/query" << std::endl;
        std::cout << "OK: " << num - ng << std::endl;
        std::cout << "NG: " << ng << std::endl;
        
        // Not support access!
        
        da.showStatus(std::cout);
        
        for (auto &str : strs)
            delete str;
    }
    
    int benchMarisa(const char* fsaName, const char *queryName, bool needsAccess = true) {
        try {
            std::cout << "Bench " << wrapper::MarisaWrapper::name() << " from " << fsaName << std::endl;
            
            wrapper::MarisaWrapper fsa(fsaName);
            
            std::cout << "Search benchmark for " << queryName << std::endl;
            
            measureBenchmark(fsa, queryName, 10, needsAccess);
            
        } catch (exception::DataNotFound e) {
            std::cerr << "Error open " << e.data_name_ << std::endl;
            return 1;
        } catch (exception::DoesntHaveMember e) {
            std::cout << "Doesn't have member: " << e.text << std::endl;
            return 1;
        }
        return 0;
    }
    
    int benchDarts(const char* fsaName, const char *queryName, bool needsAccess = true) {
        try {
            std::cout << "Bench " << wrapper::DartsCloneWrapper::name() << " from " << fsaName << std::endl;
            
            wrapper::DartsCloneWrapper darts(fsaName);
            
            std::cout << "Search benchmark for " << queryName << std::endl;
            
            measureBenchmark(darts, queryName, 10, needsAccess);
            
        } catch (exception::DataNotFound e) {
            std::cerr << "Error open " << e.data_name_ << std::endl;
            return 1;
        } catch (exception::DoesntHaveMember e) {
            std::cout << "Doesn't have member: " << e.text << std::endl;
            return 1;
        }
        return 0;
    }
    
    template <typename T>
    int bench(const char* fsa_name, const char* query_name, bool needsAccess = true) {
        try {
            std::cout << "Bench " << T::name() << " from " << fsa_name << std::endl;
            
            T fsa = benchmark::getFSAFrom<T>(fsa_name);
            
            std::cout << "Search benchmark for " << query_name << std::endl;
            
            benchmark::measureBenchmark(fsa, query_name, 10, needsAccess);
            
        } catch (exception::DataNotFound e) {
            std::cerr << "Error open " << e.data_name_ << std::endl;
            return 1;
        } catch (exception::DoesntHaveMember e) {
            std::cout << "Doesn't have member: " << e.text << std::endl;
            return 1;
        }
        return 0;
    }
    
}

int main(int argc, const char* argv[]) {
    auto fsa_name = argv[1];
    auto query_name = argv[2];
    auto type = atoi(argv[3]);
    
//    fsa_name = "../../../results/wikipedia/wikipedia.da";
//    query_name = "../../../data-sets/weiss/wikipedia.dict"; type = 12;
//    fsa_name = "../../../results/enwiki-20150205/enwiki-20150205.morfologik_cfsa2";
//    query_name = "../../../data-sets/kanda/enwiki-20150205.dict"; type = 10;
//    query_name = "../../../data-sets/weiss/wikipedia.1000000.rnd_dict"; type='9';
//    fsa_name = "../../../results/wikipedia2/wikipedia2.array_ts_fsa";
//    query_name = "../../../data-sets/weiss/wikipedia2.1000000.rnd_dict"; type='5';
//    fsa_name = "../../results/indochina-2004/indochina-2004.sac_array_fsa"; type = '6';
//    fsa_name = "../../../results/indochina-2004/indochina-2004.sample"; type = '5';
//    query_name = "../../../data-sets/kanda/indochina-2004.1000000.rnd_dict";
//    fsa_name = "../../results/jawiki-20150118/jawiki-20150118.fsa"; type = '6';
//    fsa_name = "../../../results/jawiki-20150118/jawiki-20150118.sample"; type = '5';
//    query_name = "../../../data-sets/kanda/jawiki-20150118.1000000.rnd_dict";
//    fsa_name = "../../../results/full/full.fsa"; type = '5';
//    query_name = "../../../data-sets/ciura-deorowicz/full.1000000.rnd_dict";
//    fsa_name = "../../../results/enable/enable.sample"; type = '5';
//    query_name = "../../../data-sets/ciura-deorowicz/enable.1000000.rnd_dict";
//    type = '3';
    
    switch (type) {
        case 0:
            return bench<OriginalFSA>(fsa_name, query_name);
        case 1:
            return bench<DacFSA>(fsa_name, query_name);
        case 2:
            return bench<DoubleArrayCFSA<true, true, true, true, true>>(fsa_name, query_name);
        case 3:
            return bench<DoubleArrayCFSA<false, true, true, true, true>>(fsa_name, query_name);
        case 4:
            return bench<DoubleArrayCFSA<true, false, true, true, true>>(fsa_name, query_name);
        case 5:
            return bench<DoubleArrayCFSA<true, true, false, true, true>>(fsa_name, query_name);
        case 6:
            return bench<DoubleArrayCFSA<true, true, true, false, true>>(fsa_name, query_name);
        case 11:
            return bench<DoubleArrayCFSA<true, false, true, true, false>>(fsa_name, query_name, false);
        case 7:
            return bench<wrapper::XcdatWrapper<false>>(fsa_name, query_name);
        case 8:
            return benchMarisa(fsa_name, query_name);
        case 9:
            return bench<MorfologikFSA5Dictionary>(fsa_name, query_name);
        case 10:
            return bench<MorfologikCFSA2Dictionary>(fsa_name, query_name);
        case 12:
            return benchDarts(fsa_name, query_name);
        default:
            std::cout << "type is not set!" << std::endl;
            return 1;
    }

}

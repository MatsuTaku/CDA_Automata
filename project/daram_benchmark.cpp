//
//  daram_benchmark.cpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/12/30.
//

#include "csd_automata.hpp"
#include "csd_automata/util.hpp"

namespace {

void ShowUsage() {
    std::cout
    << "Usage" << std::endl
    << "$ daram_benchmark [daram_dict] [dataset]" << std::endl;
}

template <class DaramDictionary>
    void BenchmarkDictionary(const DaramDictionary& dict, const std::string dataset_name) {
    auto keyset = csd_automata::util::GetKeySets(dataset_name);
    
#ifdef NDEBUG
    const size_t kRuns = 10;
#else
    const size_t kRuns = 1;
#endif
    
    std::cout << "Search benchmark from input: " << dataset_name << std::endl;
    std::cout << "------" << std::endl
    << "Lookup time in " << kRuns << "s runs...";
    size_t ngs = 0;
    auto lookup_time = csd_automata::util::MeasureProcessingMicro([&] {
        for (auto i = 0; i < kRuns; i++) {
            for (auto& text : keyset) {
                if (dict.Lookup(text) == 0) {
                    ngs++;
                }
            }
        }
    });
    size_t query_count = keyset.size();
    std::cout << ": " << lookup_time / kRuns / query_count << " µs/query" << std::endl
    << "OK: " << query_count - ngs / kRuns << std::endl
    << "NG: " << ngs / kRuns << std::endl << std::endl;
    
    dict.ShowStats(std::cout);
    
}

template <class Process>
    void ProcessMatchDictionary(const std::string dict_name, Process process) {
    auto dict_stream = csd_automata::util::GetStreamOrDie<std::ifstream>(dict_name);
    
    auto header = csd_automata::read_val<csd_automata::id_type>(dict_stream);
    dict_stream.seekg(0);
    
    switch (header) {
        case csd_automata::SdDaFsa::kHeader:
            process(csd_automata::SdDaFsa(dict_stream));
            break;
        case csd_automata::SdLoDaFsa::kHeader:
            process(csd_automata::SdLoDaFsa(dict_stream));
            break;
        case csd_automata::SdLoCnDaFsa::kHeader:
            process(csd_automata::SdLoCnDaFsa(dict_stream));
            break;
        case csd_automata::SdLoCidDaFsa::kHeader:
            process(csd_automata::SdLoCidDaFsa(dict_stream));
            break;
        case csd_automata::SdLoCsidDaFsa::kHeader:
            process(csd_automata::SdLoCsidDaFsa(dict_stream));
            break;
        case csd_automata::SdLoCnsidDaFsa::kHeader:
            process(csd_automata::SdLoCnsidDaFsa(dict_stream));
            break;
        case csd_automata::SdLoDwDaFsa::kHeader:
            process(csd_automata::SdLoDwDaFsa(dict_stream));
            break;
        default:
            break;
    }
}

    void Benchmark(const std::string dict_name, const std::string dataset_name) {
    ProcessMatchDictionary(dict_name, [&dataset_name](auto dict) {
        BenchmarkDictionary(dict, dataset_name);
    });
}

}

int main(int argc, char* argv[]) {
#ifdef NDEBUG
    if (argc != 3) {
        std::cout << "ERROR: Failed to invalid arguments!" << std::endl;
        ShowUsage();
        exit(EXIT_FAILURE);
    }
#endif
    
    std::string dict_name = argv[1];
    std::string dataset_name = argv[2];
    
    if (dict_name == "" or dataset_name == "") {
        std::cout << "Invalid arguments: " << dict_name << " " << dataset_name << std::endl;
        ShowUsage();
        exit(EXIT_FAILURE);
    }
    
#ifndef NDEBUG
    dict_name = "../../results/jawiki-20181001/jawiki-20181001.dam";
    dataset_name = "../../data-sets/local/jawiki-20181001.dict";
#endif
    
    Benchmark(dict_name, dataset_name);
    
    return 0;
}

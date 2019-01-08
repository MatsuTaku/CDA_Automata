//
//  Director.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/10/02.
//

#ifndef Director_hpp
#define Director_hpp

#include "basic.hpp"
#include "util.hpp"
#include "fsa_util.hpp"
#include "PlainFSABuilder.hpp"
#include "Dawg.hpp"
#include "Cdawg.hpp"
#include "ValueSet.hpp"
#include <fstream>
#include "Extension.hpp"

namespace csd_automata {
    
namespace director {

// May throw Exceptions
template <class StringDictionaryType>
int CheckHasMember(const std::string& dataset_name, StringDictionaryType& sd) {
    std::cout << std::endl << "Check membering ... ";
    auto ifs = util::GetStreamOrDie<std::ifstream>(dataset_name);
    
    auto length = 0;
    auto count = 0;
    for (std::string line; std::getline(ifs, line);) {
        count++;
        length += line.size();
        
        if (!sd.Accept(line)) {
            std::cerr << "Doesn't stored string: " << line << std::endl;
            sd.PrintForDebug(std::cerr);
            return -1;
        }
    }
    std::cout << "SUCCESS for all queries!" << std::endl
    << "Num of queries: " << count << std::endl
    << "Average query length: " << float(length) / count << std::endl;
    return 0;
}

template <class DaramType>
int FullyBuild(const std::string& out_name, const std::string& dataset_name, const std::string& values_name = "") {
    std::cout << "Input dataset: " << dataset_name << std::endl;

    // Extract base name of dataset
    std::string base_name = out_name;
    auto da_ext = extension::kExtensionDoubleArrayAutomata;
    // If baseName's extension is same as '.dam'...
    if (base_name.size() > da_ext.size() and
        base_name.substr(base_name.size() - da_ext.size()) == da_ext) {
        base_name.erase(base_name.end() - da_ext.size(), base_name.end());
    }
    
    // Union extension of plain FSA to base name
    auto plain_fsa_name = std::string(base_name) + extension::kExtensionPlainFSA;
    // Build plain FSA if needed
    std::ifstream plain_fsa_stream(plain_fsa_name);
    PlainFSA pfa;
    if (plain_fsa_stream) {
        std::cout << "Found pfsa file: " << plain_fsa_name << std::endl;
        pfa.LoadFrom(plain_fsa_stream);
    } else {
        std::cout << "Build pfa to: " << plain_fsa_name << std::endl;
        auto time_build_pfa = util::MeasureProcessing([&] {
            pfa = fsa_util::BuildPlainFSA(dataset_name);
            std::ofstream pfa_out(plain_fsa_name);
            pfa.StoreTo(pfa_out);
        });
        std::cout << "\tptime is... " << time_build_pfa << " ms"  << std::endl;
    }
    
    // Build DoubleArrayAutomata
    std::cout << "Build dam to: " << out_name << std::endl;
    
    DaramType da;
    auto time_build_dam = util::MeasureProcessing([&] {
        if (values_name == "") {
            da = DaramType(pfa);
        } else {
            auto values_stream = util::GetStreamOrDie<std::ifstream>(values_name);
            std::vector<size_t> values;
            for (std::string v; std::getline(values_stream, v);) {
                size_t vi = stoi(v);
                values.emplace_back(vi);
            }
            da = DaramType(pfa, ValueSet(values));
        }
        StoreToFile(da, out_name);
    });
    std::cout << "Build in: " << time_build_dam << " ms" << std::endl;
    
    // Check membered all sets
    if (CheckHasMember(dataset_name, da) == -1) {
        std::cerr << "Failure to build String Dictionary!" << std::endl;
    }
    
    std::cout << std::endl;
    da.ShowStats(std::cout);

    return 0;
}
    
} // namespace director
    
} // namespace csd_automata

#endif /* Director_hpp */

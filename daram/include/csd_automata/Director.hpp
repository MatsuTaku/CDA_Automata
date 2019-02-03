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
bool CheckHasMember(StringDictionaryType& sd, const std::string& dataset_name) {
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
            return false;
        }
    }
    std::cout << "SUCCESS for all queries!" << std::endl
    << "Num of queries: " << count << std::endl
    << "Average query length: " << float(length) / count << std::endl;
    return true;
}
    
template <class Daram>
Daram make_daram(const std::string& keyset_name) {
    auto pfa = fsa_util::make_plain_fsa(keyset_name);
    using Builder = typename Daram::Builder;
    Builder builder(pfa);
    return DaramType(builder);
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
            pfa = fsa_util::make_plain_fsa(dataset_name);
            std::ofstream pfa_out(plain_fsa_name);
            pfa.StoreTo(pfa_out);
        });
        std::cout << "\tptime is... " << time_build_pfa << " ms" << std::endl;
    }
    
    // Build DoubleArrayAutomataDictionary
    std::cout << "Build daram..." << std::endl;
    DaramType da;
    auto time_build_dam = util::MeasureProcessing([&] {
        using Builder = typename DaramType::Builder;
        Builder builder(pfa);
        da = DaramType(builder);
    });
    std::cout << "Build daram in: " << time_build_dam << " ms" << std::endl;
    
    // Check membered all keys
    if (not CheckHasMember(da, dataset_name)) {
        std::cerr << "Failure to build String Dictionary!" << std::endl;
    }
    
    std::cout << "Save dictionary to: " << out_name << std::endl;
    StoreToFile(da, out_name);
    
    std::cout << std::endl;
    da.ShowStats(std::cout);

    return 0;
}
    
} // namespace director
    
} // namespace csd_automata

#endif /* Director_hpp */

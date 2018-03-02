#include <iostream>
#include <unistd.h>

#include "FsaGenerator.hpp"

#include "FSA.hpp"
#include "StringTransFSA.hpp"

#include "SortedStringList.hpp"

using namespace array_fsa;

namespace  {
    void testPrefixMergeList(std::string fileName) {
        std::ifstream ifs(fileName);
        for (auto i = 0; i < 10; i++) {
            std::vector<std::string> strs;
            auto start = pow(2, i);
            auto num = 20;
            for (auto i = 0; i < start + num; i++) {
                std::string s;
                getline(ifs, s);
                if (i >= start)
                    strs.push_back(s);
            }
            SortedStringList pList;
            for (auto i = 0, c = 0; c < num; c++) {
                pList.add(strs[i], i);
                i = (i + 7) % num;
            }
            pList.showList();
            for (auto s : strs) {
                auto key = pList.lookup(s);
                if (key < 0) {
                    std::cout << "error: " << s << std::endl;
                }
                auto owner = pList.getOwnerId(s);
                if (owner > -1) {
                    std::cout << "owner: " << owner << " of " << s << std::endl;
                }
            }
        }
    }
}

int main(int argc, const char *argv[]) {
    auto data_name = argv[1];
    auto fsa_name = argv[2];
    auto fsa_type = *argv[3];
    
//    data_name = "../../data-sets/weiss/wikipedia.dict";
//    fsa_name = "../../results/wikipedia/wikipedia.fsa";
//    data_name = "../../data-sets/kanda/indochina-2004.dict";
//    fsa_name = "../../results/indochina-2004/indochina-2004.stfsa"; fsa_type = '3';
//    fsa_name = "../../results/indochina-2004/indochina-2004.stcfsa"; fsa_type = '5';
    data_name = "../../data-sets/kanda/jawiki-20150118.dict";
//    fsa_name = "../../results/jawiki-20150118/jawiki-20150118.fsa"; fsa_type = '1';
//    fsa_name = "../../results/jawiki-20150118/jawiki-20150118.stfsa"; fsa_type = '3';
    fsa_name = "../../results/jawiki-20150118/jawiki-20150118.stcfsa"; fsa_type = '6';
//    fsa_type = '3';
    
    switch (fsa_type) {
        case '1':
            return FsaGenerator<OriginalFSA>::buildFSA(data_name, fsa_name);
        case '2':
            return FsaGenerator<DacFSA>::buildFSA(data_name, fsa_name);
        case '3':
            return FsaGenerator<STFSA>::buildFSA(data_name, fsa_name);
        case '4':
            return FsaGenerator<SacFSA>::buildFSA(data_name, fsa_name);
        case '5':
            return FsaGenerator<STCFSA>::buildFSA(data_name, fsa_name);
        case '6':
            return FsaGenerator<STCFSAB>::buildFSA(data_name, fsa_name);
        default:
            return 1;
    }
     
    
    
    testPrefixMergeList(data_name);
    
}

#include <iostream>

#include "ArrayFSABuilder.hpp"
#include "ArrayDACFSABuilder.hpp"
#include "ArrayFSATailBuilder.hpp"
#include "FsaTools.hpp"
#include "PlainFSABuilder.hpp"

#include <unistd.h>

using namespace array_fsa;

namespace {
    
    class DataNotFoundException : std::exception {};
    class DoesntHaveMemberExceptipn : std::exception {
    public:
        DoesntHaveMemberExceptipn(std::string text) : text(text) {}
        std::string text;
    };
    
    PlainFSA getPlainFSAFromData(const char* data_name) {
        std::ifstream ifs(data_name);
        if (!ifs) {
            throw DataNotFoundException();
        }
        
        PlainFSABuilder builder;
        for (std::string line; std::getline(ifs, line);) {
            builder.add(line);
        }
        return builder.release();
    }
    
    template <typename FSAType>
    FSAType getArrayFSAFromData(const char* data_name) {
        PlainFSA orig_fsa = getPlainFSAFromData(data_name);
        return FSAType::Builder::build(orig_fsa);
    }

    template <typename DACFSAType>
    DACFSAType getArrayDACFSAFromData(const char* data_name, size_t dac_unit_size) {
        PlainFSA orig_fsa = getPlainFSAFromData(data_name);
        return DACFSAType::Builder::build(orig_fsa, dac_unit_size);
    }
	
    template <typename FSAType>
    void checkFsaHasMember(FSAType& fsa, const char* data_name) {
        std::ifstream ifs(data_name);
        if (!ifs) {
            throw DataNotFoundException();
        }
        
        auto num = 0;
        for (std::string line; std::getline(ifs, line);) {
            if (!FsaTools::is_member(fsa, line)) {
                throw DoesntHaveMemberExceptipn(line);
            }
            num++;
        }
    }
    
}

int main(int argc, const char* argv[]) {
    auto data_name = argv[1];
    auto fsa_name = argv[2];
    auto fsa_type = *argv[3];
    auto dac_unit_size = size_t(strtoul(argv[4], NULL, 10));
    
//    data_name = "../../data-sets/weiss/wikipedia.dict";
//    fsa_name = "../../results/wikipedia/wikipedia.array_tail_fsa";
//    fsa_type = '2';
//    dac_unit_size = 1;
    
    std::cout << "Build FSA from " << data_name << std::endl;
    
    std::ofstream ofs(fsa_name);
    if (!ofs) {
        std::cerr << "Error open " << fsa_name << std::endl;
        return 1;
    }
    
    try {
        if (fsa_type == '0') {
            ArrayFSA fsa = getArrayFSAFromData<ArrayFSA>(data_name);
            std::cout << "Test for membership" << std::endl;
            checkFsaHasMember<ArrayFSA>(fsa, data_name);
            
            std::cout << "Write FSA into " << fsa_name << std::endl;
            fsa.write(ofs);
        } else if (fsa_type == '1') {
            ArrayDACFSA fsa = getArrayDACFSAFromData<ArrayDACFSA>(data_name, dac_unit_size);
            std::cout << "Test for membership" << std::endl;
            checkFsaHasMember<ArrayDACFSA>(fsa, data_name);
            
            std::cout << "Write FSA into " << fsa_name << std::endl;
            fsa.write(ofs);
        } else if (fsa_type == '2') {
            ArrayFSATail fsa = getArrayFSAFromData<ArrayFSATail>(data_name);
            std::cout << "Test for membership" << std::endl;
            checkFsaHasMember<ArrayFSATail>(fsa, data_name);
            
            std::cout << "Write FSA into " << fsa_name << std::endl;
            fsa.write(ofs);
        }
    } catch (DataNotFoundException e) {
        std::cerr << "Error open " << data_name << std::endl;
        return 1;
    } catch (DoesntHaveMemberExceptipn e) {
        std::cout << "Doesn't have member: " << e.text << std::endl;
    }
    
	return 0;
}

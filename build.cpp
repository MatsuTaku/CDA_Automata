#include <iostream>

#include "ArrayFSABuilder.hpp"
#include "ArrayDACFSABuilder.hpp"
#include "ArrayFSATailBuilder.hpp"
#include "ArrayFSATailDACBuilder.hpp"
#include "FsaTools.hpp"
#include "PlainFSABuilder.hpp"

#include <unistd.h>

using namespace array_fsa;

namespace {
    
    class DataNotFoundException : std::exception {
    public:
        DataNotFoundException(std::string data_name) : data_name_(data_name) {}
        std::string data_name_;
    };
    class DoesntHaveMemberExceptipn : std::exception {
    public:
        DoesntHaveMemberExceptipn(std::string text) : text(text) {}
        std::string text;
    };
    
    PlainFSA getPlainFSAFromData(const char* data_name) {
        std::ifstream ifs(data_name);
        if (!ifs) {
            throw DataNotFoundException(data_name);
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
	
    template <typename FSAType>
    void checkFsaHasMember(FSAType& fsa, const char* data_name) {
        std::ifstream ifs(data_name);
        if (!ifs) {
            throw DataNotFoundException(data_name);
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
    
//    data_name = "../../data-sets/weiss/wikipedia.dict";
//    fsa_name = "../../results/wikipedia/wikipedia.array_tail_fsa";
////    data_name = "../../data-sets/ciura-deorowicz/files.dict";
////    fsa_name = "../../results/files/files.array_tail_fsa";
////    data_name = "../../data-sets/ciura-deorowicz/sample.dict";
////    fsa_name = "../../results/sample/sample.array_tail_dac_fsa";
//    fsa_type = '2';
    
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
            ArrayDACFSA fsa = getArrayFSAFromData<ArrayDACFSA>(data_name);
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
        } else if (fsa_type == '3') {
            ArrayFSATailDAC fsa = getArrayFSAFromData<ArrayFSATailDAC>(data_name);
            std::cout << "Test for membership" << std::endl;
            checkFsaHasMember<ArrayFSATailDAC>(fsa, data_name);
            
            std::cout << "Write FSA into " << fsa_name << std::endl;
            fsa.write(ofs);
        }
    } catch (DataNotFoundException e) {
        std::cerr << "Error open " << e.data_name_ << std::endl;
        return 1;
    } catch (DoesntHaveMemberExceptipn e) {
        std::cout << "Doesn't have member: " << e.text << std::endl;
    }
    
	return 0;
}

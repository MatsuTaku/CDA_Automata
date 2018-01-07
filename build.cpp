#include <iostream>
#include <unistd.h>

#include "ArrayFSABuilder.hpp"
#include "ArrayDACFSABuilder.hpp"
#include "ArrayFSATailBuilder.hpp"
#include "ArrayFSATailDACBuilder.hpp"
#include "FsaTools.hpp"
#include "PlainFSABuilder.hpp"

#include "NArrayFSA.hpp"

using namespace array_fsa;
using namespace double_array;

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
    
    PlainFSA getPlainFSAFromData(const char *data_name) {
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
    FSAType getArrayFSAFromData(const char *data_name) {
        PlainFSA orig_fsa = getPlainFSAFromData(data_name);
        return FSAType::Builder::build(orig_fsa);
    }
    
    template <typename T>
    T getNArrayFSAFromData(const char *data_name) {
        PlainFSA orig_fsa = getPlainFSAFromData(data_name);
        return T::Builder::buildN(orig_fsa);
    }
	
    template <typename FSAType>
    void checkFsaHasMember(FSAType& fsa, const char *data_name) {
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
    
    template <typename T>
    void checkWrite(T &fsa, const char *dataName, const char *fsaName, std::ofstream &ofs) {
        std::cout << "Test for membership" << std::endl;
        checkFsaHasMember<T>(fsa, dataName);
        
        std::cout << "Write FSA into " << fsaName << std::endl;
        fsa.write(ofs);
    }
    
    template <typename T>
    void buildTest(const char *dataName, const char *fsaName, std::ofstream &ofs) {
        T fsa = getArrayFSAFromData<T>(dataName);
        checkWrite(fsa, dataName, fsaName, ofs);
    }
    
}

int main(int argc, const char *argv[]) {
    auto data_name = argv[1];
    auto fsa_name = argv[2];
    auto fsa_type = *argv[3];
    
//    data_name = "../../data-sets/weiss/wikipedia.dict";
//    fsa_name = "../../results/wikipedia/wikipedia.n_array_fsa";
//    fsa_type = '5';
    
    std::cout << "Build FSA from " << data_name << std::endl;
    
    std::ofstream ofs(fsa_name);
    if (!ofs) {
        std::cerr << "Error open " << fsa_name << std::endl;
        return 1;
    }
    
    try {
        if (fsa_type == '0') {
            buildTest<ArrayFSA>(data_name, fsa_name, ofs);
        } else if (fsa_type == '1') {
            buildTest<ArrayDACFSA>(data_name, fsa_name, ofs);
        } else if (fsa_type == '2') {
            buildTest<ArrayFSATail>(data_name, fsa_name, ofs);
        } else if (fsa_type == '3') {
            buildTest<ArrayFSATailDAC>(data_name, fsa_name, ofs);
        } else if (fsa_type == '5') {
            NArrayFSA fsa = getNArrayFSAFromData<NArrayFSA>(data_name);
            checkWrite(fsa, data_name, fsa_name, ofs);
        }
    } catch (DataNotFoundException e) {
        std::cerr << "Error open " << e.data_name_ << std::endl;
        return 1;
    } catch (DoesntHaveMemberExceptipn e) {
        std::cout << "Doesn't have member: " << e.text << std::endl;
    }
    
	return 0;
}

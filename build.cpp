#include <iostream>

#include "ArrayFSABuilder.hpp"
#include "FsaTools.hpp"
#include "PlainFSABuilder.hpp"

#include <unistd.h>

using namespace array_fsa;

namespace {
    
    class DataNotFoundException : std::exception {};
    
	ArrayFSA getArrayFsaFromData(const char* data_name) {
		std::ifstream ifs(data_name);
		if (!ifs) {
			throw DataNotFoundException();
		}
		
		PlainFSABuilder builder;
		for (std::string line; std::getline(ifs, line);) {
			builder.add(line);
		}
		auto orig_fsa = builder.release();
		return ArrayFSABuilder::build(orig_fsa);
	}
	
	void checkArrayFsaHasMember(ArrayFSA& fsa, const char* data_name) {
		std::ifstream ifs(data_name);
		if (!ifs) {
			throw DataNotFoundException();
		}
		
        auto num = 0;
		for (std::string line; std::getline(ifs, line);) {
			if (!FsaTools::is_member(fsa, line)) {
                std::cout << "Doesn't have member " << line << std::endl;
			}
            num++;
		}
    }
    
}

int main(int argc, const char* argv[]) {
    auto data_name = argv[1];
    auto fsa_name = argv[2];
    
//    data_name = "../../data-sets/ciura-deorowicz/deutsch.dict";
//    fsa_name = "../../results-try/deutsch/deutsch.array_fsa";
        data_name = "../../data-sets/weiss/wikipedia.dict";
        fsa_name = "../../results-try/weiss/wikipedia.dict";
    
    std::cout << "Build FSA from " << data_name << std::endl;
    
    ArrayFSA fsa;
    try {
        fsa = getArrayFsaFromData(data_name);
    } catch (DataNotFoundException e) {
        std::cerr << "Error open " << data_name << std::endl;
        return 1;
    }
    
    std::cout << "Test for membership" << std::endl;
    checkArrayFsaHasMember(fsa, data_name);
	
	std::cout << "Write FSA into " << fsa_name << std::endl;
	
	std::ofstream ofs(fsa_name);
	if (!ofs) {
		std::cerr << "Error open " << fsa_name << std::endl;
		return 1;
	}
	fsa.write(ofs);
	
	return 0;
}

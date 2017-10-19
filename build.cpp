#include <iostream>

#include "ArrayFSABuilder.hpp"
#include "FsaTools.hpp"
#include "PlainFSABuilder.hpp"

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
		
		for (std::string line; std::getline(ifs, line);) {
			if (!FsaTools::is_member(fsa, line)) {
				throw "Doesn't have member";
			}
		}
	}
    
}

int main(int argc, const char* argv[]) {
	auto data_name = argv[1];
	auto fsa_name = argv[2];
	
	std::cout << "Build FSA from " << data_name << std::endl;
	
	ArrayFSA fsa;
	try {
		fsa = getArrayFsaFromData(data_name);
		
        std::cout << "Test for membership" << std::endl;
		checkArrayFsaHasMember(fsa, data_name);
	} catch (DataNotFoundException e) {
        std::cerr << "Error open " << data_name << std::endl;
		return 1;
	}
	
	std::cout << "Write FSA into " << fsa_name << std::endl;
	
	std::ofstream ofs(fsa_name);
	if (!ofs) {
		std::cerr << "Error open " << fsa_name << std::endl;
		return 1;
	}
	fsa.write(ofs);
	
	return 0;
}

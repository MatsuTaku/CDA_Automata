//
//  FsaGenerator.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/08.
//

#ifndef FsaGenerator_hpp
#define FsaGenerator_hpp

#include "array_fsa/PlainFSABuilder.hpp"
#include "array_fsa/PlainFSA.hpp"
#include "array_fsa/ArrayFSABuilder.hpp"
#include "array_fsa/ArrayFSATailBuilder.hpp"
#include "array_fsa/FSA.hpp"

#include "array_fsa/Exception.hpp"

namespace array_fsa {
    
    class PlainFsaGenerator {
    public:
        // May throw DataNotFoundException
        static PlainFSA generatePlainFSA(const char* dataName) {
            std::ifstream ifs(dataName);
            if (!ifs)
                throw DataNotFoundException(dataName);
            return generatePlainFSA(ifs);
        }
        
        static PlainFSA generatePlainFSA(std::ifstream& ifs) {
            PlainFSABuilder builder;
            for (std::string line; getline(ifs, line);)
                builder.add(line);
            return builder.release();
        }
        
        static PlainFSA readPlainFSA(const char* plainFsaName) {
            std::ifstream ifs(plainFsaName);
            if (!ifs) throw DataNotFoundException(plainFsaName);
            PlainFSA plainFsa;
            plainFsa.read(ifs);
            return plainFsa;
        }
        
        static void generate(const char* dataName, const char* plainFsaName) {
            auto plainFsa = generatePlainFSA(dataName);
            std::ofstream ofs(plainFsaName);
            if (!ofs)
                throw DataNotFoundException(plainFsaName);
            std::cout << "Write PlainFSA into " << plainFsaName << std::endl;
            plainFsa.write(ofs);
        }
    };
    
    template <class FSA_TYPE>
    class FsaGenerator {
    public:
        static int buildFSA(const char *dataName, const char *plainFsaName, const char *fsaName) {
            try {
                FsaGenerator<FSA_TYPE> generator;
                std::cout << "Build Array_FSA from " << plainFsaName << std::endl;
                generator.generate(plainFsaName);
                
                std::cout << "Test for membership" << std::endl;
                try {
                    generator.checkHasMember(dataName);
                } catch (DoesntHaveMemberException e) {
                    std::cout << "Doesn't have member: " << e.text << std::endl;
                    return 1;
                }
                
                std::cout << "Write FSA into " << fsaName << std::endl;
                generator.save(fsaName);
                
            } catch (DataNotFoundException e) {
                std::cerr << "Error open: " << e.data_name_ << std::endl;
                return 1;
            }
            return 0;
        }
        
        void generate(const char* plainFsaName) {
            auto plainFsa = PlainFsaGenerator::readPlainFSA(plainFsaName);
            fsa_.build(plainFsa);
        }
        
        /** May throw
        *    DataNotFoundException
        *    DoesntHaveMemberException
        */
        void checkHasMember(const char* dataName) {
            std::ifstream ifs(dataName);
            if (!ifs)
                throw DataNotFoundException(dataName);
            checkHasMember(ifs);
        }
        
        // May throw DoesntHaveMemberException
        void checkHasMember(std::ifstream& ifs) {
            auto length = 0;
            auto count = 0;
            for (std::string line; std::getline(ifs, line);) {
                count++;
                length += line.size();
                if (!fsa_.isMember(line))
                    throw DoesntHaveMemberException(line);
            }
            std::cout << "average length: " << float(length) / count << std::endl;
        }
        
        // May throw DataNotFoundException
        void save(const char* fsaName) {
            std::ofstream ofs(fsaName);
            if (!ofs)
                throw DataNotFoundException(fsaName);
            fsa_.write(ofs);
        }
        
    private:
        FSA_TYPE fsa_;
        
    };
    
}

#endif /* FsaGenerator_hpp */

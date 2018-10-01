//
//  FsaGenerator.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/08.
//

#ifndef FsaGenerator_hpp
#define FsaGenerator_hpp

#include "csd_automata/PlainFSABuilder.hpp"
#include "csd_automata/PlainFSA.hpp"
#include "csd_automata/ArrayFSABuilder.hpp"
#include "csd_automata/ArrayFSATailBuilder.hpp"
#include "csd_automata/DoubleArrayFSA.hpp"

#include "csd_automata/Exception.hpp"

namespace csd_automata {
    
    namespace PlainFsaGenerator {
        
        PlainFSA generatePlainFSA(std::ifstream &ifs) {
            PlainFSABuilder builder;
            for (std::string line; getline(ifs, line);)
                builder.add(line);
            return builder.release();
        }
        
        // May throw DataNotFoundException
        PlainFSA generatePlainFSA(const char* dataName) {
            std::ifstream ifs(dataName);
            if (!ifs)
                throw exception::DataNotFound(dataName);
            return generatePlainFSA(ifs);
        }
        
        PlainFSA readPlainFSA(const char* plainFsaName) {
            std::ifstream ifs(plainFsaName);
            if (!ifs) throw exception::DataNotFound(plainFsaName);
            PlainFSA plainFsa;
            plainFsa.read(ifs);
            return plainFsa;
        }
        
        void generate(const char* dataName, const char* plainFsaName) {
            auto plainFsa = generatePlainFSA(dataName);
            std::ofstream ofs(plainFsaName);
            if (!ofs)
                throw exception::DataNotFound(plainFsaName);
            std::cout << "Write PlainFSA into " << plainFsaName << std::endl;
            plainFsa.write(ofs);
        }
    }
    
    template <class FSA_TYPE>
    class DABuilder {
    public:
        void buildFromFSA(const char *plainFsaName) {
            auto plainFsa = PlainFsaGenerator::readPlainFSA(plainFsaName);
            fsa_.build(plainFsa);
        }
        
        /** May throw
        *    DataNotFoundException
        *    DoesntHaveMemberException
        */
        void checkHasMember(const char *dataName) {
            std::ifstream ifs(dataName);
            if (!ifs)
                throw exception::DataNotFound(dataName);
            checkHasMember(ifs);
        }
        
        // May throw Exceptions
        void checkHasMember(std::ifstream &ifs) {
            auto length = 0;
            auto count = 0;
            for (std::string line; std::getline(ifs, line);) {
                count++;
                length += line.size();
                
                if (!fsa_.isMember(line))
                    throw exception::DoesntHaveMember(line);
            }
            std::cout << "average length: " << float(length) / count << std::endl;
        }
        
        // May throw DataNotFoundException
        void save(const char *fsaName) {
            std::ofstream ofs(fsaName);
            if (!ofs)
                throw exception::DataNotFound(fsaName);
            fsa_.write(ofs);
            fsa_.showStatus(std::cout);
        }
        
    private:
        FSA_TYPE fsa_;
        
    };
    
}

#endif /* FsaGenerator_hpp */

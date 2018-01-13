//
//  FsaGenerator.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/08.
//

#ifndef FsaGenerator_hpp
#define FsaGenerator_hpp

#include "PlainFSABuilder.hpp"
#include "PlainFSA.hpp"

#include "Exception.hpp"

namespace array_fsa {
    
    template <class T>
    class FsaGenerator {
    public:
        FsaGenerator(const char* dataName) {
            generate(dataName);
        }
        
        static int buildFSA(const char *dataName, const char *fsaName) {
            try {
                std::cout << "Build FSA from " << dataName << std::endl;
                FsaGenerator<T> generator(dataName);
                
                std::cout << "Test for membership" << std::endl;
                generator.checkHasMember(dataName);
                
                std::cout << "Write FSA into " << fsaName << std::endl;
                generator.save(fsaName);
                
            } catch (DataNotFoundException e) {
                std::cerr << "Error open: " << e.data_name_ << std::endl;
                return 1;
            } catch (DoesntHaveMemberException e) {
                std::cout << "Doesn't have member: " << e.text << std::endl;
                return 1;
            }
            return 0;
        }
        
        // May throw DataNotFoundException
        PlainFSA getPlainFSA(const char* dataName) {
            std::ifstream ifs(dataName);
            if (!ifs)
                throw DataNotFoundException(dataName);
            return getPlainFSA(ifs);
        }
        
        PlainFSA getPlainFSA(std::ifstream& ifs) {
            PlainFSABuilder builder;
            for (std::string line; getline(ifs, line);)
                builder.add(line);
            return builder.release();
        }
        
        void generate(const char* dataName) {
            PlainFSA plainFsa = getPlainFSA(dataName);
            fsa_ = T::build(plainFsa);
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
            for (std::string line; std::getline(ifs, line);) {
                if (!fsa_.isMember(line))
                    throw DoesntHaveMemberException(line);
            }
        }
        
        // May throw DataNotFoundException
        void save(const char* fsaName) {
            std::ofstream ofs(fsaName);
            if (!ofs)
                throw DataNotFoundException(fsaName);
            fsa_.write(ofs);
        }
        
    private:
        T fsa_;
    };
    
}

#endif /* FsaGenerator_hpp */

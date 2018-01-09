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

#include "FsaTools.hpp"
#include "Exception.hpp"

namespace array_fsa {
    
    class FsaGenerator {
    public:
        FsaGenerator() = delete;
        
        static PlainFSA getPlainFSA(const char *queryName) {
            PlainFSABuilder builder;
            
            const auto strs = KeySet::getKeySets(queryName);
            for (const auto& str : strs)
                builder.add(str);
            return builder.release();
        }
        
        template <class T>
        static T generateFSA(const char *queryName) {
            PlainFSA plainFsa = getPlainFSA(queryName);
            return T::build(plainFsa);
        }
        
        template <class T>
        static void checkHasMember(const T &fsa, const char *dataName) {
            const auto strs = KeySet::getKeySets(dataName);
            
            for (const auto& str : strs) {
                if (!fsa.isMember(str))
                    throw DoesntHaveMemberException(str);
            }
        }
        
        template <class T>
        static void saveFSA(const T &fsa, const char *fsaName) {
            std::ofstream ofs(fsaName);
            if (!ofs)
                throw DataNotFoundException(fsaName);
            fsa.write(ofs);
        }
    };
    
}

#endif /* FsaGenerator_hpp */

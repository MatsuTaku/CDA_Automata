//
// Created by Kampersanda on 2017/05/28.
//

#ifndef ARRAY_FSA_FSATOOLS_HPP
#define ARRAY_FSA_FSATOOLS_HPP

#include "basic.hpp"

#include "PlainFSABuilder.hpp"
#include "ArrayFSABuilder.hpp"
#include "ArrayFSATailBuilder.hpp"

namespace array_fsa {
    
    class DataNotFoundException : std::exception {
    public:
        DataNotFoundException(const std::string &data_name) : data_name_(data_name) {}
        std::string data_name_;
    };
    
    class DoesntHaveMemberExceptipn : std::exception {
    public:
        DoesntHaveMemberExceptipn(const std::string &text) : text(text) {}
        std::string text;
    };
    
    class FsaTools {
    public:
        static PlainFSA getPlainFSA(const char *queryName) {
            PlainFSABuilder builder;
            
            std::vector<std::string> strs;
            setKeySets(strs, queryName);
            for (auto line : strs)
                builder.add(line);
            return builder.release();
        }
        
        template <class T>
        static T generateFSA(const char *queryName) {
            PlainFSA plainFsa = getPlainFSA(queryName);
            return T::Builder::template build<T>(plainFsa);
        }
        
        template <typename T>
        static T getFSAFrom(const char *fsaName) {
            std::ifstream ifs(fsaName);
            if (!ifs) {
                std::cerr << "Error open " << fsaName << std::endl;
                throw DataNotFoundException(fsaName);
            }
            T fsa;
            fsa.read(ifs);
            return fsa;
        }
        
        template <typename T>
        static void checkHasMember(T &fsa, const char *dataName) {
            std::vector<std::string> strs;
            setKeySets(strs, dataName);
            
            for (auto line : strs) {
                if (!fsa.isMember(line)) {
                    throw DoesntHaveMemberExceptipn(line);
                }
            }
        }
        
        template <typename T>
        static void measureBenchmark(T &fsa, const char *queryName) {
            std::vector<std::string> strs;
            setKeySets(strs, queryName);
            
            size_t ok = 0, ng = 0;
            std::vector<std::string> ngs;
            auto chars = 0;
            
            Stopwatch sw;
            for (const auto& str : strs) {
                if (fsa.isMember(str)) {
                    ++ok;
                    chars += str.length();
                } else {
                    ++ng;
                    ngs.push_back(str);
                }
            }
            auto mSec = sw.get_micro_sec();
            
            std::cout << "avg length: " << float(chars) / strs.size() << std::endl;
            
            std::cout << "Lookup time: " << mSec / (ok + ng) << " us/query" << std::endl;
            std::cout << "OK: " << ok << std::endl;
            std::cout << "NG: " << ng << std::endl;
            for (auto s : ngs) {
                std::cout << s << std::endl;
            }
            
            fsa.show_stat(std::cout);
        }
        
        template <class T>
        static void saveFSA(T &fsa, const char *fsaName) {
            std::ofstream ofs(fsaName);
            if (!ofs)
                throw DataNotFoundException(fsaName);
            fsa.write(ofs);
        }
        
        static void setKeySets(std::vector<std::string> &strs, const char *queryName) {
            std::ifstream ifs(queryName);
            if (!ifs)
                throw DataNotFoundException(queryName);
            
            for (std::string line; std::getline(ifs, line);) {
                strs.push_back(line);
            }
            strs.shrink_to_fit();
        }
        
    };
    
}

#endif //ARRAY_FSA_FSATOOLS_HPP

//
//  Director.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/10/02.
//

#ifndef Director_hpp
#define Director_hpp

#include "basic.hpp"
#include "PlainFSABuilder.hpp"
#include "DoubleArrayFSA.hpp"
#include "DoubleArrayCFSA.hpp"
#include "ValueSet.hpp"
#include <fstream>
#include "Exception.hpp"
#include "Extension.hpp"

namespace csd_automata {
    
    namespace director {
        
        /**
         Measurement iput process as milli sec
         */
        double measureProcessing(std::function<void(void)> prc) {
            Stopwatch sw;
            prc();
            return sw.get_milli_sec();
        }
        
        // Might throw DataNotFoundException
        PlainFSA buildPlainFSA(const std::string &dataName) {
            std::ifstream ifs(dataName);
            if (!ifs)
                throw exception::DataNotFound(dataName);
            
            PlainFSABuilder builder;
            for (std::string line; getline(ifs, line);)
                builder.add(line);
            
            return builder.release();
        }
        
        PlainFSA readPlainFSA(const std::string &plainFsaName) {
            std::ifstream ifs(plainFsaName);
            if (!ifs)
                throw exception::DataNotFound(plainFsaName);
            PlainFSA plainFsa;
            plainFsa.read(ifs);
            return plainFsa;
        }
        
        void generate(const std::string &dataName, const std::string &plainFsaName) {
            auto plainFsa = buildPlainFSA(dataName);
            std::ofstream ofs(plainFsaName);
            if (!ofs)
                throw exception::DataNotFound(plainFsaName);
            std::cout << "Write PlainFSA into " << plainFsaName << std::endl;
            plainFsa.write(ofs);
        }
        
        // May throw Exceptions
        template <class SD_TYPE>
        void checkHasMember(std::string datasetName, SD_TYPE& sd) {
            std::ifstream ifs(datasetName);
            if (!ifs)
                throw exception::DataNotFound(datasetName);
            
            auto length = 0;
            auto count = 0;
            for (std::string line; std::getline(ifs, line);) {
                count++;
                length += line.size();
                
                if (!sd.isMember(line))
                    throw exception::DoesntHaveMember(line);
            }
            std::cout << "Check membering all queries success!" << std::endl
            << "Num of queries: " << count << std::endl
            << "Average query length: " << float(length) / count << std::endl;
        }
        
        template <class DA_TYPE>
        int fullyBuild(const std::string& outName, const std::string& datasetName, const std::string& valuesName = "") {
            std::cout << "Input dataset: " << datasetName << std::endl;
            
            try {
                double buildTime = measureProcessing([&]() {
                    // Extract base name of dataset
                    std::string baseName = outName;
                    auto daExe = extension::DoubleArrayAutomataExtension;
                    if (baseName.size() > daExe.size() &&
                        baseName.substr(baseName.size() - daExe.size()) == daExe) {
                        baseName.erase(baseName.end() - daExe.size(), baseName.end());
                    }
                    
                    // Union extension of plain FSA to base name
                    auto plainFSAName = std::string(baseName) + extension::PlainFSAExtension;
                    // Build plain FSA if needed
                    std::ifstream plainFsaStream(plainFSAName);
                    PlainFSA pfa;
                    if (plainFsaStream) {
                        std::cout << "Found pfsa file: " << plainFSAName << std::endl;
                        pfa.read(plainFsaStream);
                    } else {
                        std::cout << "Build pfa to: " << plainFSAName << std::endl;
                        auto timeBuildPFA = measureProcessing([&]() {
                            try {
                                pfa = buildPlainFSA(datasetName);
                            } catch (exception::DataNotFound& e) {
                                std::cerr << e.what() << std::endl;
                                throw;
                            }
                            std::ofstream pfaOut(plainFSAName);
                            pfa.write(pfaOut);
                        });
                        std::cout << "\tptime is... " << timeBuildPFA << " ms"  << std::endl;
                    }
                    
                    // Build DoubleArrayAutomata
                    std::cout << "Build dam to: " << outName << std::endl;
                    
                    DA_TYPE da;
                    auto timeBuildDAM = measureProcessing([&]() {
                        if (valuesName == "") {
                            da = DA_TYPE(pfa);
                        } else {
                            std::ifstream valuesStream(valuesName);
                            if (!valuesStream)
                                throw exception::DataNotFound(valuesName);
                            std::vector<size_t> values;
                            for (std::string v; std::getline(valuesStream, v);) {
                                size_t vi = stoi(v);
                                values.emplace_back(vi);
                            }
                            da = DA_TYPE(pfa, ValueSet(values));
                        }
                        std::ofstream outStream(outName);
                        da.write(outStream);
                    });
                    std::cout << "\ttime is... " << timeBuildDAM << " ms" << std::endl;
                    
                    // Check membered all sets
                    try {
                        checkHasMember(datasetName, da);
                    } catch (exception::DataNotFound& e) {
                        std::cerr << e.what() << std::endl;
                        throw;
                    } catch (exception::DoesntHaveMember& e) {
                        std::cerr << e.what() << std::endl;
                        throw;
                    }
                    
                    da.showStatus(std::cout);
                });
                
                std::cout << std::endl << "Build in: " << buildTime / 1000 << " s" << std::endl;
                
            } catch (std::exception& e) {
                return -1;
            }
            
            return 0;
        }
        
    } // namespace director
    
} // namespace csd_automata

#endif /* Director_hpp */

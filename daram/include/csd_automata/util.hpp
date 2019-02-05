//
//  util.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/12/30.
//

#ifndef util_hpp
#define util_hpp

#include "basic.hpp"

namespace csd_automata::util {

class Stopwatch {
    using clock = std::chrono::high_resolution_clock;
public:
    Stopwatch() : start_(clock::now()) {}
    
    double get_sec() const {
        auto tp = clock::now() - start_;
        return std::chrono::duration<double>(tp).count();
    }
    double get_milli_sec() const {
        auto tp = clock::now() - start_;
        return std::chrono::duration<double, std::milli>(tp).count();
    }
    double get_micro_sec() const {
        auto tp = clock::now() - start_;
        return std::chrono::duration<double, std::micro>(tp).count();
    }
    
private:
    clock::time_point start_;
};

/**
 Measurement input process as milli sec
 */
template <class Process>
inline double MeasureProcessing(Process process) {
    Stopwatch sw;
    process();
    return sw.get_milli_sec();
}

/**
 Measurement input process as micro sec
 */
template <class Process>
inline double MeasureProcessingMicro(Process process) {
    Stopwatch sw;
    process();
    return sw.get_micro_sec();
}

template <class Stream>
inline Stream GetStreamOrDie(const std::string& file_name) {
    Stream stream(file_name);
    if (!stream) {
        std::cout << "ERROR: Not found file: " << file_name << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return stream;
}

std::vector<std::string> GetKeySets(const std::string queryName) {
    auto ifs = GetStreamOrDie<std::ifstream>(queryName);
    std::vector<std::string> strs;
    for (std::string line; std::getline(ifs, line);)
        strs.push_back(line);
    return strs;
}

}

#endif /* util_hpp */

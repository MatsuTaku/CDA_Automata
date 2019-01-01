//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_BASIC_HPP
#define ARRAY_FSA_BASIC_HPP

#include <array>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <stdlib.h>
#include <stdint.h>
#include <cmath>
#include <memory>
#include <sstream>
#include <cstring>
#include <fstream>
#include <limits>
#include <chrono>

#include "sim_ds/basic.hpp"

namespace csd_automata {


using id_type = sim_ds::id_type;



template <typename T>
inline T read_val(std::istream& is) {
    T val;
    is.read(reinterpret_cast<char*>(&val), sizeof(val));
    return val;
}

template <typename T>
inline std::vector<T> read_vec(std::istream& is) {
    auto size = read_val<size_t>(is);
    std::vector<T> vec(size);
    is.read(reinterpret_cast<char*>(&vec[0]), sizeof(T) * size);
    return vec; // expect move
}

inline bool read_bit(std::istream& is) {
    return static_cast<bool>(read_val<uint8_t>(is));
}

template <typename T>
inline void write_val(const T& val, std::ostream& os) {
    os.write(reinterpret_cast<const char*>(&val), sizeof(val));
}

template <typename T>
inline void write_vec(const std::vector<T>& vec, std::ostream& os) {
    write_val(vec.size(), os);
    os.write(reinterpret_cast<const char*>(&vec[0]), sizeof(T) * vec.size());
}

inline void write_bit(const bool bit, std::ostream& os) {
    write_val(static_cast<uint8_t>(bit), os);
}

template <typename T>
inline size_t size_vec(const std::vector<T>& vec) {
    return sizeof(T) * vec.size() + sizeof(vec.size());
}

struct MorfologikFileUtils {
    static int32_t read_int(std::istream& is) {
        int32_t v = 0;
        for(int i = 0; i < 4; i++) {
            v = v << 8 | (read_val<uint8_t>(is) & 0xFF);
        }
        return v;
    }
    
    static int16_t read_short(std::istream& is) {
        int16_t byte = read_val<int8_t>(is);
        return (byte << 8) | read_val<int8_t>(is);
    }
    
    static std::vector<uint8_t> fully_read(std::istream& is) {
        const auto offset = is.tellg();
        is.seekg(0, is.end);
        const auto end = is.tellg();
        is.seekg(offset);
        
        std::vector<uint8_t> ret(static_cast<size_t>(end - offset));
        is.read(reinterpret_cast<char*>(&ret[0]), sizeof(uint8_t) * ret.size());
        
        return ret;
    }
};

struct FsaHeader {
    uint8_t version;
    uint8_t filler;
    uint8_t annotation;
    uint8_t gtl;
};

inline FsaHeader read_fsa_header(std::istream& is) {
    auto magic = MorfologikFileUtils::read_int(is);
    if (0x5c667361 != magic) {
        std::cerr << "Invalid file header magic bytes." << std::endl;
        exit(1);
    }
    
    auto read_uint8 = read_val<uint8_t>;
    
    return FsaHeader {
        read_uint8(is),
        read_uint8(is),
        read_uint8(is),
        read_uint8(is)
    };
}

inline const char* bool_str(bool b) {
    return b ? "T" : "F";
}

}

#endif //ARRAY_FSA_BASIC_HPP

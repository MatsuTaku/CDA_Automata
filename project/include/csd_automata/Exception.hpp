//
//  Exception.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/08.
//

#ifndef Exception_hpp
#define Exception_hpp

#include "basic.hpp"

namespace csd_automata {
    
namespace exception {
    
class DataNotFound : std::exception {
public:
    DataNotFound(const std::string& data_name) : data_name_(data_name) {}
    
    const char* what() const throw() override {
        return (std::string("Data not found!: ") + data_name_).c_str();
    }
    
private:
    std::string data_name_;
};

class DoesntHaveMember : std::exception {
public:
    DoesntHaveMember(const std::string &text) : text(text) {}
    
    const char* what() const throw() override {
        return (std::string("Not membered!: ") + text).c_str();
    }
    
private:
    std::string text;
};


class LookupError : std::exception {
public:
    LookupError(const std::string &key, size_t value, size_t incollect) : key(key), value(value), incollect(incollect) {}
    
    const char* what() const throw() override {
        return (std::string("Lookup error: [ \"") + key + "\": " + std::to_string(value) + " ] ^ " + std::to_string(incollect)).c_str();
    }
    
private:
    std::string key;
    size_t value, incollect;
};


class AccessError : std::exception {
public:
    AccessError(const std::string &key, size_t value, const std::string &incollect) : key(key), value(value), incollect(incollect) {}
    
    const char* what() const throw() override {
        return (std::string("Access error: [\"") + key + "\": " + std::to_string(value) + " ] ^ \"" + incollect + '"').c_str();
    }
    
private:
    std::string key;
    size_t value;
    std::string incollect;
};
    
}
    
}

#endif /* Exception_hpp */

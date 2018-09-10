//
//  Exception.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/08.
//

#ifndef Exception_hpp
#define Exception_hpp

#include "basic.hpp"

namespace array_fsa {
    
    class DataNotFoundException : std::exception {
    public:
        DataNotFoundException(const std::string &data_name) : data_name_(data_name) {}
        std::string data_name_;
    };
    
    class DoesntHaveMemberException : std::exception {
    public:
        DoesntHaveMemberException(const std::string &text) : text(text) {}
        std::string text;
    };
    
    using std::cerr, std::endl;
    
    class LookupErrorException : std::exception {
    public:
        LookupErrorException(const std::string &key, size_t value, size_t incollect) : key(key), value(value), incollect(incollect) {}
        std::string key;
        size_t value, incollect;
        
        void error()  {
            cerr << "Lookup error: [ \"" << key << "\": " << value << " ] ^ " << incollect << endl;
        }
    };
    
    class AccessErrorException : std::exception {
    public:
        AccessErrorException(const std::string &key, size_t value, const std::string &incollect) : key(key), value(value), incollect(incollect) {}
        std::string key;
        size_t value;
        const std::string &incollect;
        
        void error()  {
            cerr << "Access error: [\"" << key << "\": " << value << " ] ^ \"" << incollect << '"' << endl;
        }
    };
    
}

#endif /* Exception_hpp */

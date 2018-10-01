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
            DataNotFound(const std::string &data_name) : data_name_(data_name) {}
            std::string data_name_;
        };
        
        class DoesntHaveMember : std::exception {
        public:
            DoesntHaveMember(const std::string &text) : text(text) {}
            std::string text;
        };
        
        using std::cerr, std::endl;
        
        class LookupError : std::exception {
        public:
            LookupError(const std::string &key, size_t value, size_t incollect) : key(key), value(value), incollect(incollect) {}
            std::string key;
            size_t value, incollect;
            
            void error()  {
                cerr << "Lookup error: [ \"" << key << "\": " << value << " ] ^ " << incollect << endl;
            }
        };
        
        class AccessError : std::exception {
        public:
            AccessError(const std::string &key, size_t value, const std::string &incollect) : key(key), value(value), incollect(incollect) {}
            std::string key;
            size_t value;
            const std::string &incollect;
            
            void error()  {
                cerr << "Access error: [\"" << key << "\": " << value << " ] ^ \"" << incollect << '"' << endl;
            }
        };
        
    }
    
}

#endif /* Exception_hpp */

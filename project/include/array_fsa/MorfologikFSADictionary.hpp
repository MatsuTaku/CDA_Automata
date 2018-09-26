//
//  MorfologikFSA5Dictionary.hpp
//  ArrayFSA_dev
//
//  Created by 松本拓真 on 2018/09/24.
//

#ifndef MorfologikFSADictionary_hpp
#define MorfologikFSADictionary_hpp

#include "MorfologikFSA5DictionaryFoundation.hpp"

namespace array_fsa {
    
    template<class Foundation>
    class MorfologikFSADictionary {
    public:
        MorfologikFSADictionary(Foundation &fd) : fd_(fd) {}
        
        MorfologikFSADictionary(std::istream &is) : fd_(Foundation(is)) {}
        
        ~MorfologikFSADictionary() = default;
        
        MorfologikFSADictionary(const MorfologikFSADictionary &) = delete;
        MorfologikFSADictionary &operator=(const MorfologikFSADictionary &) = delete;
        
        MorfologikFSADictionary(MorfologikFSADictionary &&) = default;
        MorfologikFSADictionary &operator=(MorfologikFSADictionary &&) = default;
        
        // MARK: String-Dictionary's functions
        
        static std::string name() {
            return "MorfologikFSA5Dictionary";
        }
        
        template<typename S>
        bool isMember(const S &str) const;
        
        template<typename S>
        size_t lookup(const S &str) const;
        
        std::string access(size_t id) const;
        
        size_t getNumTrans() const {
            fd_.numTrans();
        }
        
        size_t sizeInBytes() const {
            return fd_.sizeInBytes();
        }
        
        void showStatus(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << fd_.numTrans() << endl;
            os << "size:   " << sizeInBytes() << endl;
        }
        
        void printForDebug(std::ostream& os) const {
            fd_.printForDebug(os);
        }
        
        // MARK: IO
        
        void write(std::ostream &os) const {
            fd_.write(os);
        }
        
    private:
        const Foundation fd_;
        
    };
    
    
    template<class Foundation>
    template<typename S>
    inline bool MorfologikFSADictionary<Foundation>::isMember(const S &str) const {
        size_t state = fd_.getRootState(), arc = 0;
        for (uint8_t c : str) {
            arc = fd_.getTrans(state, c);
            if (arc == 0) {
                std::cerr << "Error not membered: " << str << std::endl;
                return false;
            }
            
            state = fd_.getTargetState(arc);
        }
        
        return fd_.isFinalTrans(arc);
    }
    
    template<class Foundation>
    template<typename S>
    inline size_t MorfologikFSADictionary<Foundation>::lookup(const S &str) const {
        size_t words = 0;
        
        size_t state = fd_.getRootState(), arc = 0;
        for (uint8_t c : str) {
            for (arc = fd_.getFirstTrans(state); arc != 0 && fd_.getTransSymbol(arc) != c; arc = fd_.getNextTrans(arc)) {
                words += fd_.getTransWords(arc);
            }
            if (arc == 0) {
                std::cerr << "Error not membered: " << str << std::endl;
                return 0;
            }
            
            if (fd_.isFinalTrans(arc))
                words++;
            
            state = fd_.getTargetState(arc);
        }
        
        return fd_.isFinalTrans(arc) ? words : -1;
    }
    
    template<class Foundation>
    inline std::string MorfologikFSADictionary<Foundation>::access(size_t id) const {
        std::string str = "";
        
        size_t counter = id;
        for (size_t state = fd_.getRootState(), arc = 0; counter > 0; state = fd_.getTargetState(arc)) {
            for (arc = fd_.getFirstTrans(state); arc != 0 && counter > 0; arc = fd_.getNextTrans(arc)) {
                auto words = fd_.getTransWords(arc);
                if (words < counter) {
                    counter -= words;
                } else {
                    if (fd_.isFinalTrans(arc))
                        counter--;
                    str += fd_.getTransSymbol(arc);
                    break;
                }
            }
            if (arc == 0) {
                std::cerr << "Error not stored id: " << id << std::endl;
                return "";
            }
        }
        
        return str;
    }
    
}

#endif /* MorfologikFSADictionary_hpp */

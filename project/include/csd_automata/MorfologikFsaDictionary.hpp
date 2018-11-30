//
//  MorfologikFsa5Dictionary.hpp
//  ArrayFSA_dev
//
//  Created by 松本拓真 on 2018/09/24.
//

#ifndef MorfologikFsaDictionary_hpp
#define MorfologikFsaDictionary_hpp

#include "MorfologikFSA5DictionaryFoundation.hpp"
#include "MorfologikCFSA2DictionaryFoundation.hpp"

namespace csd_automata {
    
template<class FoundationType>
class MorfologikFsaDictionary {
    
    const FoundationType fd_;
    
public:
    using Foundation = FoundationType;
    
    MorfologikFsaDictionary(Foundation&& fd) : fd_(std::move(fd)) {}
    
    MorfologikFsaDictionary(std::istream& is) : fd_(Foundation(is)) {}
    
    // MARK: String-Dictionary's functions
    
    static std::string name() {
        return "MorfologikFSA5Dictionary";
    }
    
    /**
     Check is a string stored.

     @param str String to check
     @return Boolean that a string is stored or not
     */
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
    
    void ShowStatus(std::ostream& os) const {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "#trans: " << fd_.numTrans() << endl;
        os << "size:   " << sizeInBytes() << endl;
    }
    
    void PrintForDebug(std::ostream& os) const {
        fd_.PrintForDebug(os);
    }
    
    // MARK: IO
    
    void write(std::ostream &os) const {
        fd_.write(os);
    }
    
    MorfologikFsaDictionary() = default;
    ~MorfologikFsaDictionary() = default;
    
    MorfologikFsaDictionary(const MorfologikFsaDictionary&) = delete;
    MorfologikFsaDictionary& operator=(const MorfologikFsaDictionary&) = delete;
    
    MorfologikFsaDictionary(MorfologikFsaDictionary&&) = default;
    MorfologikFsaDictionary& operator=(MorfologikFsaDictionary&&) = default;
    
    
};


template<class FoundationType>
template<typename S>
inline bool MorfologikFsaDictionary<FoundationType>::isMember(const S& str) const {
    size_t state = fd_.getRootState(), trans = 0;
    for (uint8_t c : str) {
        trans = fd_.getTrans(state, c);
        if (trans == 0) {
            std::cerr << "Error not membered: " << str << std::endl;
            return false;
        }
        
        state = fd_.getTargetState(trans);
    }
    
    return fd_.isFinalTrans(trans);
}

template<class FoundationType>
template<typename S>
inline size_t MorfologikFsaDictionary<FoundationType>::lookup(const S& str) const {
    size_t words = 0;
    
    size_t state = fd_.getRootState(), trans = 0;
    for (uint8_t c : str) {
        for (trans = fd_.getFirstTrans(state); trans != 0 && fd_.getTransSymbol(trans) != c; trans = fd_.getNextTrans(trans)) {
            words += fd_.getTransWords(trans);
        }
        if (trans == 0) {
            std::cerr << "Error not membered: " << str << std::endl;
            return 0;
        }
        
        if (fd_.isFinalTrans(trans))
            words++;
        
        state = fd_.getTargetState(trans);
    }
    
    return fd_.isFinalTrans(trans) ? words : -1;
}

template<class FoundationType>
inline std::string MorfologikFsaDictionary<FoundationType>::access(size_t id) const {
    std::string str = "";
    
    size_t counter = id;
    for (size_t state = fd_.getRootState(), trans = 0; counter > 0; state = fd_.getTargetState(trans)) {
        for (trans = fd_.getFirstTrans(state); trans != 0 && counter > 0; trans = fd_.getNextTrans(trans)) {
            auto words = fd_.getTransWords(trans);
            if (words < counter) {
                counter -= words;
            } else {
                if (fd_.isFinalTrans(trans))
                    counter--;
                str += fd_.getTransSymbol(trans);
                break;
            }
        }
        if (trans == 0) {
            std::cerr << "Error not stored id: " << id << std::endl;
            return "";
        }
    }
    
    return str;
}



using MorfologikFSA5Dictionary = MorfologikFsaDictionary<MorfologikFSA5DictionaryFoundation>;
using MorfologikCFSA2Dictionary = MorfologikFsaDictionary<MorfologikCFSA2DictionaryFoundation>;
    
}

#endif /* MorfologikFSADictionary_hpp */

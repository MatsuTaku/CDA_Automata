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
    
    static std::string name() {
        return Foundation::name();
    }
    
    MorfologikFsaDictionary(Foundation&& fd) : fd_(std::move(fd)) {}
    
    MorfologikFsaDictionary(std::istream& is) : fd_(Foundation(is)) {}
    
    // MARK: String-Dictionary's functions
    
    /**
     Check is a string stored.

     @param str String to check
     @return Boolean that a string is stored or not
     */
    template<typename S>
    bool Accept(const S &str) const;
    
    template<typename S>
    size_t Lookup(const S &str) const;
    
    std::string Access(size_t id) const;
    
    size_t get_num_trans() const {
        fd_.numTrans();
    }
    
    size_t size_in_bytes() const {
        return fd_.size_in_bytes();
    }
    
    void ShowStats(std::ostream& os) const {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "#trans: " << fd_.num_trans() << endl;
        os << "size:   " << size_in_bytes() << endl;
    }
    
    void PrintForDebug(std::ostream& os) const {
        fd_.PrintForDebug(os);
    }
    
    // MARK: IO
    
    void Write(std::ostream &os) const {
        fd_.Write(os);
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
inline bool MorfologikFsaDictionary<FoundationType>::Accept(const S& str) const {
    size_t state = fd_.get_root_state(), trans = 0;
    for (uint8_t c : str) {
        trans = fd_.get_trans(state, c);
        if (trans == 0) {
            std::cerr << "Error not membered: " << str << std::endl;
            return false;
        }
        
        state = fd_.get_target_state(trans);
    }
    
    return fd_.is_final_trans(trans);
}

template<class FoundationType>
template<typename S>
inline size_t MorfologikFsaDictionary<FoundationType>::Lookup(const S& str) const {
    size_t words = 0;
    
    size_t state = fd_.get_root_state(), trans = 0;
    for (uint8_t c : str) {
        for (trans = fd_.get_first_trans(state); trans != 0 && fd_.get_trans_symbol(trans) != c; trans = fd_.get_next_trans(trans)) {
            words += fd_.get_trans_words(trans);
        }
        if (trans == 0) {
            std::cerr << "Error not membered: " << str << std::endl;
            return 0;
        }
        
        if (fd_.is_final_trans(trans))
            words++;
        
        state = fd_.get_target_state(trans);
    }
    
    return fd_.is_final_trans(trans) ? words : -1;
}

template<class FoundationType>
inline std::string MorfologikFsaDictionary<FoundationType>::Access(size_t id) const {
    std::string str = "";
    
    size_t counter = id;
    for (size_t state = fd_.get_root_state(), trans = 0; counter > 0; state = fd_.get_target_state(trans)) {
        for (trans = fd_.get_first_trans(state); trans != 0 && counter > 0; trans = fd_.get_next_trans(trans)) {
            auto words = fd_.get_trans_words(trans);
            if (words < counter) {
                counter -= words;
            } else {
                if (fd_.is_final_trans(trans))
                    counter--;
                str += fd_.get_trans_symbol(trans);
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



using SdMrfFsa5 = MorfologikFsaDictionary<MorfologikFSA5DictionaryFoundation>;
using SdMrfCfsa2 = MorfologikFsaDictionary<MorfologikCFSA2DictionaryFoundation>;
    
}

#endif /* MorfologikFSADictionary_hpp */

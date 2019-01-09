//
//  MorfologikFsa5Dictionary.hpp
//  ArrayFSA_dev
//
//  Created by 松本拓真 on 2018/09/24.
//

#ifndef MorfologikFsaDictionary_hpp
#define MorfologikFsaDictionary_hpp

#include "StringDictionaryInterface.hpp"
#include "IOInterface.hpp"
#include "MorfologikFSA5DictionaryFoundation.hpp"
#include "MorfologikCFSA2DictionaryFoundation.hpp"

namespace csd_automata {
    
template<class MorfologikFsaBaseType>
class MorfologikFsaDictionary : public StringDictionaryInterface, public IOInterface, MorfologikFsaBaseType {
public:
    using Base = MorfologikFsaBaseType;
    using FsaSource = typename MorfologikFsaBaseType::FsaSource;
    
    static std::string name() {
        return Base::name();
    }
    
    static std::string tag() {
        return (std::string("Morfologik")
                + (typeid(MorfologikFsaBaseType) == typeid(MorfologikCFSA2DictionaryFoundation) ? "-c" : ""));
    }
    
    MorfologikFsaDictionary() = default;
    
    MorfologikFsaDictionary(std::istream& is) noexcept : Base(is) {}
    
    MorfologikFsaDictionary(const FsaSource& fsa) noexcept : Base(fsa) {}
    
    MorfologikFsaDictionary(Base&& fd) noexcept : Base(std::move(fd)) {}
    
    // MARK: String-Dictionary's functions
    
    /**
     Check is a string stored.

     @param str String to check
     @return Boolean that a string is stored or not
     */
    bool Accept(std::string_view str) const override;
    
    id_type Lookup(std::string_view str) const override;
    
    std::string Access(id_type id) const override;
    
    size_t size_in_bytes() const override {
        return Base::size_in_bytes();
    }
    
    void LoadFrom(std::istream& is) override {
        Base::LoadFrom(is);
    }
    
    void StoreTo(std::ostream& os) const override {
        Base::StoreTo(os);
    }
    
    void ShowStats(std::ostream& os) const override {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "#trans: " << Base::num_trans() << endl;
        os << "size:   " << Base::size_in_bytes() << endl;
    }
    
    void PrintForDebug(std::ostream& os) const {
        Base::PrintForDebug(os);
    }
    
    
};


template <class FoundationType>
inline bool MorfologikFsaDictionary<FoundationType>::Accept(std::string_view str) const {
    size_t state = Base::get_root_state(), trans = 0;
    for (uint8_t c : str) {
        trans = Base::get_trans(state, c);
        if (trans == 0) {
            std::cerr << "Error not membered: " << str << std::endl;
            return false;
        }
        
        state = Base::get_target_state(trans);
    }
    
    return Base::is_final_trans(trans);
}

template <class FoundationType>
inline id_type MorfologikFsaDictionary<FoundationType>::Lookup(std::string_view str) const {
    size_t words = 0;
    
    size_t state = Base::get_root_state(), trans = 0;
    for (uint8_t c : str) {
        for (trans = Base::get_first_trans(state);
             trans != 0 and Base::get_trans_symbol(trans) != c;
             trans = Base::get_next_trans(trans))
        {
            words += Base::get_trans_words(trans);
        }
        if (trans == 0) {
            std::cerr << "Error not membered: " << str << std::endl;
            return 0;
        }
        
        if (Base::is_final_trans(trans))
            words++;
        
        state = Base::get_target_state(trans);
    }
    
    return Base::is_final_trans(trans) ? words : 0;
}

template <class FoundationType>
inline std::string MorfologikFsaDictionary<FoundationType>::Access(id_type id) const {
    std::string str = "";
    
    size_t counter = id;
    for (size_t state = Base::get_root_state(), trans = 0;
         counter > 0;
         state = Base::get_target_state(trans))
    {
        for (trans = Base::get_first_trans(state);
             trans != 0 and counter > 0;
             trans = Base::get_next_trans(trans))
        {
            auto words = Base::get_trans_words(trans);
            if (words < counter) {
                counter -= words;
            } else {
                if (Base::is_final_trans(trans))
                    counter--;
                str += Base::get_trans_symbol(trans);
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
using SdMrfCFsa2 = MorfologikFsaDictionary<MorfologikCFSA2DictionaryFoundation>;

}

#endif /* MorfologikFSADictionary_hpp */

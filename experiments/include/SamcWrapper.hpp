//
//  SamcWrapper.hpp
//
//  Created by 松本拓真 on 2019/05/27.
//

#ifndef SamcWrapper_hpp
#define SamcWrapper_hpp

#include "sim_ds/Samc.hpp"
#include "csd_automata/util.hpp"

namespace wrapper {

class SamcWrapper {
    using Samc = sim_ds::SamcDict<uint32_t>;
    
public:
    static std::string name() {
        return typeid(Samc).name();
    }
    
    static std::string tag() {
        return std::string("samc");
    }
    
    SamcWrapper() = default;
    
    explicit SamcWrapper(std::istream& is) {
        Samc::input_trie<char, uint32_t> trie;
        for (std::string s; std::getline(is, s);) {
            trie.insert(s, 1);
        }
        auto build_time = csd_automata::util::MeasureProcessingMicro([&]() {
            samc_ = Samc(trie);
        });
        std::cout << "\tBuild time: " << std::fixed << std::setprecision(2) << build_time / 1000000 << " s" << std::endl;
    }
    
    friend void LoadFromFile(SamcWrapper& self, std::string file_name);
    
    bool Accept(std::string_view str) const {
        return samc_.lookup(str) != Samc::kSearchError;
    }
    
    size_t Lookup(std::string_view str) const {
        return samc_.lookup(str);
    }
    
    std::string Access(size_t value) const {
        return samc_.access(value);
    }
    
    size_t size_in_bytes() const {
        return samc_.size_in_bytes();
    }
    
    void Write(std::ostream& os) const {
        samc_.Write(os);
    }
    
    void Read(std::istream& is) {
        samc_.Read(is);
    }
    
    void ShowStats(std::ostream& os) const {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
//        os << "#nodes:   " << num_nodes() << endl;
        os << "size:   " << size_in_bytes() << endl;
    }
    
    void PrintForDebug(std::ostream &os) const {
        
    }
    
private:
    Samc samc_;
    
};


void LoadFromFile(SamcWrapper& self, std::string file_name) {
    auto ifs = csd_automata::util::GetStreamOrDie<std::ifstream>(file_name);
    self.Read(ifs);
}

}

#endif /* SamcWrapper_hpp */

//
//  CommonPrefixSet.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/11/13.
//

#ifndef CommonPrefixSet_hpp
#define CommonPrefixSet_hpp

#include "basic.hpp"
#include "sim_ds/BitVector.hpp"
#include "sim_ds/SuccinctBitVector.hpp"

namespace csd_automata {
    
class CommonPrefixSet {
private:
    std::string base_string_;
    sim_ds::SuccinctBitVector<> terminals_;
    std::vector<size_t> ids_;
    // For build
    sim_ds::BitVector b_terminals_;
    
public:
    CommonPrefixSet(std::string_view str) : base_string_(str) {}
    
    std::string_view operator[](size_t index) const {
        return std::string_view(base_string_.data(), terminals_.select(index) + 1);
    }
    
    void AppendPrefixAndId(size_t length, size_t id) {
        if (length == 0 or length > base_string_.size())
            return;
        if (terminals_.size() < length)
            b_terminals_.resize(length);
        b_terminals_[length - 1] = true;
        ids_.emplace_back(id);
    }
    
    void Freeze() {
        terminals_ = sim_ds::SuccinctBitVector<>(b_terminals_);
        b_terminals_.resize(0);
        b_terminals_.shrink_to_fit();
    }
    
    /**
     Call argument function for each prefixes:
     
     @param each function which has arguments:
       number: curret number of prefix,
       prefix: prefix,
       id    : mapped ID from prefix
     */
    template <class Func>
    void for_each(Func each) const {
        int count = 0;
        for (int i = 0; i < terminals_.size(); i++) {
            if (terminals_[i]) {
                each(count, std::string_view(base_string_.data(), i + 1), ids_[count]);
                count++;
            }
        }
    }
    
};
    
}

#endif /* CommonPrefixSet_hpp */

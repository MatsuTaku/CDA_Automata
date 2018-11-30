//
//  CommonPrefixSet.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/11/13.
//

#ifndef CommonPrefixSet_hpp
#define CommonPrefixSet_hpp

#include "sim_ds/BitVector.hpp"

namespace csd_automata {
    
class CommonPrefixSet {
private:
    std::string base_string_;
    sim_ds::BitVector terminals_;
    std::vector<size_t> ids_;
    
public:
    CommonPrefixSet(const std::string& str) : base_string_(str) {}
    
    std::string_view operator[](size_t index) const {
        return std::string_view(base_string_.data(), terminals_.select(index) + 1);
    }
    
    void AppendPrefix(size_t length, size_t id) {
        if (length == 0 || length > base_string_.size())
            return;
        if (terminals_.size() < length)
            terminals_.resize(length);
        terminals_[length - 1] = true;
        terminals_.Build(true);
        ids_.emplace_back(id);
    }
    
    template <class Func>
    void for_each(Func Each) const {
        int count = 0;
        for (int i = 0; i < terminals_.size(); i++) {
            if (terminals_[i]) {
                Each(count, std::string_view(base_string_.data(), i + 1), ids_[count]);
                count++;
            }
        }
    }
    
};
    
}

#endif /* CommonPrefixSet_hpp */

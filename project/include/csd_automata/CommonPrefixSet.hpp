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

namespace csd_automata {
    
    class CommonPrefixSet {
    private:
        std::string base_string_;
        sim_ds::BitVector terminals_;
        std::vector<size_t> ids_;
        
    public:
        CommonPrefixSet(const std::string& str) : base_string_(str) {}
        
        std::string_view operator[](size_t index) const {
            int count = 0;
            for (int i = 0; i < terminals_.size(); i++) {
                if (terminals_[i]) {
                    if (count == index)
                        return std::string_view(base_string_.data(), i + 1);
                    count++;
                }
            }
            return "";
        }
        
        void appendPrefix(size_t length, size_t id) {
            if (length == 0 || length > base_string_.size())
                return;
            terminals_.set(length - 1, true);
            ids_.emplace_back(id);
        }
        
        template<class Func>
        void forEach(Func each) const {
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

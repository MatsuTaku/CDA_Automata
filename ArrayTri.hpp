//
//  ArrayTri.hpp
//  bench
//
//  Created by 松本拓真 on 2017/11/13.
//

#ifndef ArrayTri_hpp
#define ArrayTri_hpp

#include "basic.hpp"

namespace array_fsa {
    
    // Dynamic Tri Tree as 2 dimensions array.
    class ArrayTri {
    public:
        ArrayTri() {
            expand(1);
        }
        
        void add(std::string &text, size_t index);
        size_t isMember(std::string &text);
        
    private:
        std::vector<std::vector<size_t>> bytes_;
        std::vector<size_t> checks_;
        std::vector<size_t> finishes_;
        
        size_t findNext(size_t index, uint8_t symbol);
        void expand(size_t size);
        
    };
    
    // MARK: - public
    
    inline void ArrayTri::add(std::string &text, size_t index) {
        size_t trans = 0;
        for (uint8_t c : text) {
            auto next = bytes_[trans][c];
            if (next == 0) {
                next = findNext(trans, c);
                bytes_[trans][c] = next;
                checks_[next] = trans;
            }
            trans = next;
        }
        finishes_[trans] = index;
    }
    
    inline size_t ArrayTri::isMember(std::string &text) {
        size_t trans = 0;
        for (auto c : text) {
            auto target = bytes_[trans][c];
            if (target >= bytes_.size() || checks_[target] != trans || target == 0) {
                return false;
            }
            trans = target;
        }
        return finishes_[trans];
    }
    
    // MARK: - private
    
    inline size_t ArrayTri::findNext(size_t index, uint8_t symbol) {
        expand(bytes_.size() + 1);
        return bytes_.size() - 1;
    }
    
    inline void ArrayTri::expand(size_t size) {
        std::vector<size_t> zeros;
        zeros.resize(0x100, 0);
        bytes_.resize(size, zeros);
        checks_.resize(size, 0);
        finishes_.resize(size, 0);
    }
    
}

#endif /* ArrayTri_hpp */

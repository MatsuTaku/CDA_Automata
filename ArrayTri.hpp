//
//  ArrayTri.hpp
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
        
        void add(const std::string &text, size_t index);
        int getFinishTrans(const std::string& text) const;
        int lookup(const std::string& text) const;
        int getOwnerId(const std::string& text) const;
        
    private:
        static constexpr int kFailed = -1;
        
        std::vector<std::vector<size_t>> bytes_;
        std::vector<size_t> checks_;
        std::vector<int> finishes_;
        std::vector<bool> has_state_;
        
        size_t findNext(size_t index, uint8_t symbol);
        void expand(size_t size);
        
    };
    
    // MARK: - public
    
    inline void ArrayTri::add(const std::string &text, size_t index) {
        size_t trans = 0;
        for (uint8_t c : text) {
            auto next = bytes_[trans][c];
            if (next == 0) {
                next = findNext(trans, c);
                bytes_[trans][c] = next;
                checks_[next] = trans;
                has_state_[trans] = true;
            }
            trans = next;
        }
        finishes_[trans] = index;
    }
    
    inline int ArrayTri::getFinishTrans(const std::string &text) const {
        size_t trans = 0;
        for (uint8_t c : text) {
            auto target = bytes_[trans][c];
            auto overflow = target >= bytes_.size();
            auto failedTrans = checks_[target] != trans;
            auto isNotExist = target == 0;
            if (overflow || failedTrans || isNotExist)
                return kFailed;
            trans = target;
        }
        return trans;
    }
    
    inline int ArrayTri::lookup(const std::string &text) const {
        auto trans = getFinishTrans(text);
        return trans != kFailed ? finishes_[trans] : kFailed;
    }
    
    inline int ArrayTri::getOwnerId(const std::string &text) const {
        size_t trans = getFinishTrans(text);
        if (trans == kFailed)
            return kFailed;
        bool updated = false;
        while (has_state_[trans]) {
            for (auto num : bytes_[trans]) {
                if (num != 0) {
                    trans = num;
                    updated = true;
                    break;
                }
            }
        }
        return updated ? finishes_[trans] : kFailed;
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
        finishes_.resize(size, kFailed);
        has_state_.resize(size, false);
    }
    
}

#endif /* ArrayTri_hpp */

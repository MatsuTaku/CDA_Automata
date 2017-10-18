//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_ARRAYFSABUILDER_HPP
#define ARRAY_FSA_ARRAYFSABUILDER_HPP

#include <unordered_map>

#include "ArrayFSA.hpp"

namespace array_fsa {
    
    class PlainFSA;
    class ArrayFSABuilder {
    public:
        static constexpr size_t kAddrSize = 4;
        static constexpr size_t kElemSize = 1 + kAddrSize * 2;
        
        static ArrayFSA build(const PlainFSA& orig_fsa);
        
        ArrayFSABuilder(const ArrayFSABuilder&) = delete;
        ArrayFSABuilder& operator=(const ArrayFSABuilder&) = delete;
        
    private:
        static constexpr size_t kBlockSize = 0x100;
        static constexpr size_t kFreeBytes = 0x10 * kBlockSize * kElemSize; // like darts-clone
        
        const PlainFSA& orig_fsa_;
        
        std::vector<uint8_t> bytes_;
        std::unordered_map<size_t, size_t> state_map_;
        size_t unfrozen_head_ = 0;
        
        explicit ArrayFSABuilder(const PlainFSA& orig_fsa) : orig_fsa_(orig_fsa) {}
        ~ArrayFSABuilder() = default;
        
        // MARK: of array
        size_t index_(size_t offset) const {
            return offset / kElemSize;
        }
        // MARK: of codes
        size_t offset_(size_t index) const {
            return index * kElemSize;
        }
        
        // MARK: Getters
        bool is_final_(size_t offset) const {
            return (bytes_[offset] & 1) == 1;
        }
        bool is_frozen_(size_t offset) const {
            return (bytes_[offset] & 2) == 2;
        }
        bool is_used_next_(size_t offset) const {
            return (bytes_[offset] & 4) == 4;
        }
        size_t get_next_(size_t offset) const {
            size_t next = 0;
            std::memcpy(&next, &bytes_[offset + 1], kAddrSize);
            return next;
        }
        uint8_t get_check_(size_t offset) const {
            return bytes_[offset + 1 + kAddrSize];
        }
        size_t get_succ_(size_t offset) const {
            size_t succ = 0;
            std::memcpy(&succ, &bytes_[offset + 1], kAddrSize);
            return succ;
        }
        size_t get_pred_(size_t offset) const {
            size_t pred = 0;
            std::memcpy(&pred, &bytes_[offset + 1 + kAddrSize], kAddrSize);
            return pred;
        }
        
        // MARK: Setters
        void set_next_(size_t offset, size_t next) {
            std::memcpy(&bytes_[offset + 1], &next, kAddrSize);
        }
        void set_check_(size_t offset, uint8_t check) {
            bytes_[offset + 1 + kAddrSize] = check;
        }
        void set_succ_(size_t offset, size_t succ) {
            std::memcpy(&bytes_[offset + 1], &succ, kAddrSize);
        }
        void set_pred_(size_t offset, size_t pred) {
            std::memcpy(&bytes_[offset + 1 + kAddrSize], &pred, kAddrSize);
        }
        
        // MARK: methods
        
        void build_();
        
        void expand_();
        
        void freeze_state_(size_t offset);
        
        void close_block_(size_t begin);
        
        void arrange_(size_t state, size_t offset);
        
        // so-called XCHECK
        size_t find_next_(size_t first_trans) const;
        
        bool check_next_(size_t next, size_t trans) const;
    };
    
}

#endif //ARRAY_FSA_ARRAYFSABUILDER_HPP

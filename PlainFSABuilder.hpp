//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_FSABUILDER_HPP
#define ARRAY_FSA_FSABUILDER_HPP

#include "PlainFSA.hpp"

namespace array_fsa {
    
    class PlainFSABuilder {
    public:
        static constexpr size_t kBufferGrowthSize = 0x100 * PlainFSA::kTransSize;
        
        PlainFSABuilder();
        
        ~PlainFSABuilder() = default;
        
        void add(const std::string& str);
        PlainFSA release();
        
        PlainFSABuilder(const PlainFSABuilder&) = delete;
        PlainFSABuilder& operator=(const PlainFSABuilder&) = delete;
        
    private:
        struct Range {
            size_t begin;
            size_t end;
            size_t length() const {
                return end - begin;
            }
            
            void press_size(size_t size) {
                begin += size;
            }
            void append_size(size_t size) {
                end += size;
            }
            void close_to_end() {
                end = begin;
            }
        };
        
        std::vector<uint8_t> bytes_; // serialized FSA
        
        std::vector<Range> active_path_;
        size_t active_len_ = 0;
        
        std::vector<size_t> register_; // hash table
        size_t num_registered_ = 0;
        
        bool is_last_trans_(size_t trans) const {
            return (bytes_[trans] & 1) != 0;
        }
        bool is_final_trans_(size_t trans) const {
            return (bytes_[trans] & 2) != 0;
        }
        void set_final_flag_(size_t trans, bool is_final) {
            if (is_final) { bytes_[trans + 0] |= 2; }
            else { bytes_[trans + 0] &= ~2; }
        }
        uint8_t get_symbol_(size_t trans) const {
            return bytes_[trans + 1];
        }
        void set_symbol_(size_t trans, char symbol) {
            bytes_[trans + 1] = static_cast<uint8_t>(symbol);
        }
        size_t get_target_(size_t trans) const {
            size_t target = 0;
            std::memcpy(&target, &bytes_[trans + 2], PlainFSA::kAddrSize);
            return target;
        }
        void set_target_(size_t arc, size_t target) {
            std::memcpy(&bytes_[arc + 2], &target, PlainFSA::kAddrSize);
        }
        bool is_invalid_trans_(size_t trans) const {
            return (bytes_[trans] & 0x80) != 0;
        }
        void set_invalid_flag_(size_t trans) {
            bytes_[trans] |= 0x80;
        }
        void clear_trans_(size_t trans) {
            bytes_[trans] = static_cast<uint8_t>(0);
        }
        
        size_t get_lcp_(const std::string& str) const;
        size_t freeze_state_(const Range& range);
        
        size_t hash_(Range range) const;
        size_t serialize_(const Range& range);
        bool equivalent_(size_t lhs_begin, Range rhs) const;
        
        size_t allocate_state_(size_t num_trans);
        
        Range get_range_(size_t begin) const;
        
        void expand_buffers_();
        void expand_active_path_(size_t len);
        void expand_register_();
    };
    
}

#endif //ARRAY_FSA_FSABUILDER_HPP

//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_PLAINFSA_HPP
#define ARRAY_FSA_PLAINFSA_HPP

#include "basic.hpp"

namespace array_fsa {
    
    class PlainFSA {
        friend class PlainFSABuilder;
        
    public:
        static constexpr size_t kAddrSize = 4;
        static constexpr size_t kTransSize = 2 + kAddrSize;
        
        PlainFSA() = default;
        ~PlainFSA() = default;
        
        size_t get_root_state() const {
            return get_target_state(0);
        }
        
        size_t get_trans(size_t state, uint8_t symbol) const {
            for (auto t = get_first_trans(state); t != 0; t = get_next_trans(t)) {
                if (symbol == get_trans_symbol(t)) {
                    return t;
                }
            }
            return 0;
        }
        
        size_t get_target_state(size_t trans) const {
            size_t target = 0;
            std::memcpy(&target, &bytes_[trans + 2], kAddrSize);
            return target;
        }
        
        bool is_final_trans(size_t trans) const {
            return (bytes_[trans] & 2) != 0;
        }
        
        size_t get_first_trans(size_t state) const {
            return state;
        }
        
        size_t get_next_trans(size_t trans) const {
            return is_last_trans(trans) ? 0 : trans + kTransSize;
        }
        
        bool is_last_trans(size_t trans) const {
            return (bytes_[trans] & 1) != 0;
        }
        
        bool is_multi_src_state(size_t state) const {
            return (bytes_[get_first_trans(state)] & 4) != 0;
        }
        
        uint8_t get_trans_symbol(size_t trans) const {
            return bytes_[trans + 1];
        }
        
        size_t get_num_trans() const {
            return num_trans_;
        }
        
        size_t get_num_elements() const {
            return bytes_.size() / kTransSize;
        }
        
        bool is_multi_child_state(size_t state) const {
            auto target = get_target_state(state);
            return !is_final_trans(state) && !is_last_trans(target);
        }
        
        bool is_straight_state(size_t state) const {
            auto isSingleSrc = !is_multi_src_state(state);
            auto isLessSingleChild = !is_multi_child_state(state);
            auto isFinal = is_final_trans(state);
            auto isStraight = isSingleSrc && isLessSingleChild && !isFinal;
            return isStraight;
        }
        
        void print_for_debug(std::ostream& os) const {
            using std::endl;
            
            os << "\tS\tF\tL\tM\tP" << endl;
            for (size_t i = 0; i < bytes_.size();) {
                if ((bytes_[i] & 0x80) != 0) {
                    i += kTransSize * 0x100;
                    continue;
                }
                os << i << "\t"
                << get_trans_symbol(i) << "\t"
                << is_final_trans(i) << "\t"
                << is_last_trans(i) << "\t"
                << is_multi_src_state(i) << "\t"
                << get_target_state(i) << endl;
                i += kTransSize;
            }
        }
        
        void swap(PlainFSA& rhs) {
            bytes_.swap(rhs.bytes_);
            std::swap(num_trans_, rhs.num_trans_);
        }
        
        PlainFSA(const PlainFSA&) = delete;
        PlainFSA& operator=(const PlainFSA&) = delete;
        
        PlainFSA(PlainFSA&& rhs) noexcept : PlainFSA() {
            this->swap(rhs);
        }
        PlainFSA& operator=(PlainFSA&& rhs) noexcept {
            this->swap(rhs);
            return *this;
        }
        
    protected:
        std::vector<uint8_t> bytes_; // serialized FSA
        size_t num_trans_ = 0;
        
    };
    
}

#endif //ARRAY_FSA_PLAINFSA_HPP

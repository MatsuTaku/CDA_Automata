//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_PLAINFSA_HPP
#define ARRAY_FSA_PLAINFSA_HPP

#include "basic.hpp"

namespace csd_automata {
    
class PlainFSA {
    friend class PlainFSABuilder;
public:
    static constexpr size_t kSizeAddr = 4;
    static constexpr size_t kSizeTrans = 2 + kSizeAddr * 2;
    
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
        std::memcpy(&target, &bytes_[trans + 2], kSizeAddr);
        return target;
    }
    
    bool is_final_trans(size_t trans) const {
        return static_cast<bool>(bytes_[trans] & 2);
    }
    
    size_t get_words_trans(size_t trans) const {
        size_t store = 0;
        std::memcpy(&store, &bytes_[trans + 2 + kSizeAddr], kSizeAddr);
        return store;
    }
    
    size_t get_first_trans(size_t state) const {
        return state;
    }
    
    size_t get_next_trans(size_t trans) const {
        return is_last_trans(trans) ? 0 : trans + kSizeTrans;
    }
    
    bool is_last_trans(size_t trans) const {
        return static_cast<bool>(bytes_[trans] & 1);
    }
    
    bool is_multi_src_state(size_t state) const {
        return static_cast<bool>(bytes_[get_first_trans(state)] & 4);
    }
    
    uint8_t get_trans_symbol(size_t trans) const {
        return bytes_[trans + 1];
    }
    
    size_t get_num_trans() const {
        return num_trans_;
    }
    
    size_t get_num_elements() const {
        return bytes_.size() / kSizeTrans;
    }
    
    size_t get_num_words() const {
        return num_words_;
    }
    
    bool is_multi_child_state(size_t state) const {
        auto target = get_target_state(state);
        return !is_final_trans(state) && !is_last_trans(target);
    }
    
    bool is_straight_state(size_t state) const {
        auto is_single_src = !is_multi_src_state(state);
        auto is_less_single_child = !is_multi_child_state(state);
        auto is_final = is_final_trans(state);
        auto is_single_node = is_single_src && is_less_single_child && !is_final;
        return is_single_node;
    }
    
    template <class Work>
    void ForAllSymbolInFollowsTrans(size_t trans, bool* break_flag, Work work) const {
        for (size_t t = trans; !*break_flag && t != 0; t = get_next_trans(t)) {
            work(get_trans_symbol(t));
        }
    }
    
    template <class Work>
    void ForAllSymbolInState(size_t state, bool* break_flag, Work work) const {
        ForAllSymbolInFollowsTrans(get_first_trans(state), break_flag, work);
    }
    
    void print_for_debug(std::ostream& os, size_t startIndex = 0) const {
        using std::endl;
        const char tab = '\t';
        
        os << "\tS\tF\tL\tM\tP" << endl;
        for (size_t i = startIndex; i < bytes_.size();) {
            if (static_cast<bool>(bytes_[i] & 0x80)) {
                i += kSizeTrans * 0x100;
                continue;
            }
            os << tab << i << tab
            << get_trans_symbol(i) << tab
            << is_final_trans(i) << tab
            << is_last_trans(i) << tab
            << is_multi_src_state(i) << tab
            << get_target_state(i) << endl;
            i += kSizeTrans;
        }
    }
    
    void swap(PlainFSA& rhs) {
        bytes_.swap(rhs.bytes_);
        std::swap(num_trans_, rhs.num_trans_);
        std::swap(num_words_, rhs.num_words_);
    }
    
    void LoadFrom(std::istream &is) {
        bytes_ = read_vec<uint8_t>(is);
        num_trans_ = read_val<size_t>(is);
        num_words_ = read_val<size_t>(is);
    }
    
    void StoreTo(std::ostream& os) const {
        write_vec(bytes_, os);
        write_val(num_trans_, os);
        write_val(num_words_, os);
    }
    
    PlainFSA() = default;
    ~PlainFSA() = default;
    
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
    size_t num_words_ = 0;
    
};
    
}

#endif //ARRAY_FSA_PLAINFSA_HPP

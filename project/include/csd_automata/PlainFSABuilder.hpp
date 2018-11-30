//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_FSABUILDER_HPP
#define ARRAY_FSA_FSABUILDER_HPP

#include "PlainFSA.hpp"

namespace csd_automata {
    
class PlainFSABuilder {
public:
    static constexpr size_t kBufferGrowthSize = 0x100 * PlainFSA::kTransSize;
    
    PlainFSABuilder();
    ~PlainFSABuilder() = default;
    
    void add(const std::string &str);
    PlainFSA release();
    
    PlainFSABuilder(const PlainFSABuilder&) = delete;
    PlainFSABuilder& operator=(const PlainFSABuilder&) = delete;
    
protected:
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
    
    size_t num_words_ = 0;
    
    bool is_last_trans_(size_t trans) const {
        return static_cast<bool>(bytes_[trans] & 1);
    }
    bool is_final_trans_(size_t trans) const {
        return static_cast<bool>(bytes_[trans] & 2);
    }
    uint8_t get_symbol_(size_t trans) const {
        return bytes_[trans + 1];
    }
    size_t get_target_(size_t trans) const {
        size_t target = 0;
        std::memcpy(&target, &bytes_[trans + 2], PlainFSA::kAddrSize);
        return target;
    }
    size_t get_store_(size_t trans) const {
        size_t store = 0;
        std::memcpy(&store, &bytes_[trans + 2 + PlainFSA::kAddrSize], PlainFSA::kAddrSize);
        return store;
    }
    bool is_invalid_trans_(size_t trans) const {
        return static_cast<bool>(bytes_[trans] & 0x80);
    }
    void set_final_flag_(size_t trans, bool is_final) {
        if (is_final) { bytes_[trans + 0] |= 2; }
        else { bytes_[trans + 0] &= ~2; }
    }
    void set_symbol_(size_t trans, char symbol) {
        bytes_[trans + 1] = static_cast<uint8_t>(symbol);
    }
    void set_target_(size_t arc, size_t target) {
        std::memcpy(&bytes_[arc + 2], &target, PlainFSA::kAddrSize);
    }
    void set_store_(size_t arc, size_t store) {
        std::memcpy(&bytes_[arc + 2 + PlainFSA::kAddrSize], &store, PlainFSA::kAddrSize);
    }
    void set_is_multi_src_state_(size_t trans, bool is_multi_src) {
        if (is_multi_src) { bytes_[trans] |= 4; }
        else { bytes_[trans] &= ~4; }
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


// MARK: - constructor

PlainFSABuilder::PlainFSABuilder() {
    register_.resize(1U << 10, 0);
    allocate_state_(1);
    bytes_[0] = 1; // set last flag
    set_symbol_(0, '^');
    expand_active_path_(1);
}


// MARK: - public

void PlainFSABuilder::add(const std::string& str) {
    const auto lcp = get_lcp_(str);
    expand_active_path_(str.length());
    
    if (active_len_ > 0) { // After first addition
        // minimize
        for (size_t i = active_len_ - 1; i > lcp; --i) {
            const auto state = freeze_state_(active_path_[i]);
            set_target_(active_path_[i - 1].end - PlainFSA::kTransSize, state);
            active_path_[i].close_to_end();
        }
    }
    
    for (auto i = lcp; i < str.length(); ++i) {
        const auto trans = active_path_[i].end;
        clear_trans_(trans);
        const bool is_final = i + 1 == str.length();
        set_final_flag_(trans, is_final);
        set_symbol_(trans, str[i]);
        set_target_(trans, !is_final ? active_path_[i + 1].begin : 0);
        set_store_(trans, 0);
        active_path_[i].end = trans + PlainFSA::kTransSize;
    }
    
    // Set store, that is number of suffixes exist after transition.
    auto counter = 0;
    for (auto i = 0; i < str.length(); i++) {
        const auto trans = active_path_[i].end - PlainFSA::kTransSize;
        const auto store = get_store_(trans);
        if (store > 0 && store == counter)
            break;
        set_store_(trans, store + 1);
        counter = store + 1;
    }
    
    active_len_ = str.length();
    
    num_words_++;
    
}

PlainFSA PlainFSABuilder::release() {
    std::string empty_string;
    add(empty_string);
    
    if (active_path_[0].length() == 0) {
        set_target_(0, 0);
    } else {
        set_target_(0, freeze_state_(active_path_[0]));
    }
    
    // for debug print
    for (auto path : active_path_) {
        set_invalid_flag_(path.begin);
    }
    
    // detect duplicating targets
    std::vector<bool> flags(bytes_.size() / PlainFSA::kTransSize, false);
    for (size_t i = 0; i < bytes_.size();) {
        if (is_invalid_trans_(i)) {
            i += PlainFSA::kTransSize * 0x100;
            continue;
        }
        auto target = get_target_(i);
        auto flags_target = target / PlainFSA::kTransSize;
        if (!flags[flags_target]) {
            flags[flags_target] = true;
        } else {
            set_is_multi_src_state_(target, true);
        }
        
        i += PlainFSA::kTransSize;
    }
    
    PlainFSA fsa;
    fsa.num_trans_ = bytes_.size() / PlainFSA::kTransSize - active_path_.size() * 0x100;
    fsa.bytes_ = std::move(bytes_);
    fsa.num_words_ = num_words_;
    
    return fsa;
}


// MARK: - private

size_t PlainFSABuilder::get_lcp_(const std::string& str) const {
    const auto len = std::min(str.length(), active_len_);
    for (size_t i = 0; i < len; ++i) {
        const auto trans = active_path_[i].end - PlainFSA::kTransSize;
        if (static_cast<uint8_t>(str[i]) != get_symbol_(trans)) {
            return i;
        }
    }
    return len;
}

size_t PlainFSABuilder::freeze_state_(const Range& range) {
    bytes_[range.end - PlainFSA::kTransSize + 0] |= 1; // set last flag
    
    const auto bucket_mask = register_.size() - 1;
    auto slot = hash_(range) & bucket_mask;
    
    for (size_t i = 1;; ++i) {
        auto state = register_[slot];
        if (state == 0) {
            // insert the state to register
            state = serialize_(range);
            register_[slot] = state;
            if (++num_registered_ > register_.size() / 2) {
                expand_register_();
            }
            return state;
        }
        if (equivalent_(state, range)) {
            return state;
        }
        slot = (slot + i) & bucket_mask;
    }
}

size_t PlainFSABuilder::hash_(Range range) const {
    size_t h = 0;
    while (range.length() > 0) {
        h = 17 * h + get_symbol_(range.begin);
        h = 17 * h + get_target_(range.begin);
        if (is_final_trans_(range.begin)) {
            h += 17;
        }
        range.press_size(PlainFSA::kTransSize);
    }
    return h;
}
size_t PlainFSABuilder::serialize_(const Range& range) {
    const auto new_state = bytes_.size();
    auto len = range.length();
    bytes_.resize(new_state + len);
    std::memcpy(&bytes_[new_state], &bytes_[range.begin], len);
    
    return new_state;
}
bool PlainFSABuilder::equivalent_(size_t lhs_begin, Range rhs) const {
    const auto len = rhs.length();
    if (lhs_begin + len > bytes_.size()) {
        return false;
    }
    return std::memcmp(&bytes_[lhs_begin], &bytes_[rhs.begin], len) == 0;
}

size_t PlainFSABuilder::allocate_state_(size_t num_trans) {
    expand_buffers_();
    
    const auto new_state = bytes_.size();
    bytes_.resize(new_state + num_trans * PlainFSA::kTransSize);
    return new_state;
}

PlainFSABuilder::Range PlainFSABuilder::get_range_(size_t begin) const {
    Range range { begin, begin };
    while (!is_last_trans_(range.end)) {
        range.append_size(PlainFSA::kTransSize);
    }
    range.append_size(PlainFSA::kTransSize);
    return range;
}

void PlainFSABuilder::expand_buffers_() {
    if (bytes_.capacity() < bytes_.size() + kBufferGrowthSize) {
        bytes_.reserve(bytes_.capacity() + kBufferGrowthSize);
    }
}

void PlainFSABuilder::expand_active_path_(size_t len) {
    if (active_path_.size() >= len) {
        return;
    }
    const auto trans = active_path_.size();
    active_path_.resize(len);
    for (auto i = trans; i < len; ++i) {
        const auto state = allocate_state_(0x100);
        active_path_[i] = { state, state };
    }
}

void PlainFSABuilder::expand_register_() {
    std::vector<size_t> new_register(register_.size() * 2, 0);
    const auto bucket_mask = new_register.size() - 1;
    
    // rehash
    for (size_t slot = 0; slot < register_.size(); ++slot) {
        const auto state = register_[slot];
        if (state == 0) {
            continue;
        }
        auto new_slot = hash_(get_range_(state)) & bucket_mask;
        for (size_t i = 1;; ++i) {
            if (new_register[new_slot] == 0) {
                break;
            }
            new_slot = (new_slot + i) & bucket_mask;
        }
        new_register[new_slot] = state;
    }
    
    register_ = std::move(new_register);
}
    
}

#endif //ARRAY_FSA_FSABUILDER_HPP

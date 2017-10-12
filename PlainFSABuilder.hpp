//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_FSABUILDER_HPP
#define ARRAY_FSA_FSABUILDER_HPP

#include "PlainFSA.hpp"

namespace array_fsa {

class PlainFSABuilder {
public:
  static constexpr size_t kBufferGrowthSize = 256 * PlainFSA::kTransSize;

  PlainFSABuilder() {
    register_.resize(1U << 10, 0);
    allocate_state_(1);
    bytes_[0] = 1; // set last flag
    bytes_[1] = '^';
    expand_active_path_(1);
  }

  ~PlainFSABuilder() = default;

  void add(const std::string& str) {
    const auto lcp = get_lcp_(str);
    expand_active_path_(str.length());

    if (active_len_ > 0) {
      // minimize
      for (size_t i = active_len_ - 1; i > lcp; --i) {
        const auto state = freeze_state_(active_path_[i]);
        set_target_(active_path_[i - 1].end - PlainFSA::kTransSize, state);
        active_path_[i].end = active_path_[i].begin;
      }
    } else {
      // first addition
    }

    for (auto i = lcp; i < str.length(); ++i) {
      const auto trans = active_path_[i].end;
      bytes_[trans + 0] = static_cast<uint8_t>(i + 1 != str.length() ? 0 : 2); // set final flag
      bytes_[trans + 1] = static_cast<uint8_t>(str[i]); // set symbol
      set_target_(trans, i + 1 != str.length() ? active_path_[i + 1].begin : 0);
      active_path_[i].end = trans + PlainFSA::kTransSize;
    }

    active_len_ = str.length();
  }

  PlainFSA release() {
    {
      std::string empty_str;
      add(empty_str);
    }

    if (active_path_[0].begin == active_path_[0].end) {
      set_target_(0, 0);
    } else {
      set_target_(0, freeze_state_(active_path_[0]));
    }

    // for debug print
    for (size_t i = 0; i < active_path_.size(); ++i) {
      bytes_[active_path_[i].begin + 0] |= 128; // set invalid flag
    }

    { // detect duplicating targets
      std::vector<bool> flags(bytes_.size() / PlainFSA::kTransSize, false);
      for (size_t i = 0; i < bytes_.size();) {
        if ((bytes_[i] & 128) != 0) {
          i += PlainFSA::kTransSize * 256;
          continue;
        }
        auto target = get_target_(i);
        if (!flags[target / PlainFSA::kTransSize]) {
          flags[target / PlainFSA::kTransSize] = true;
        } else {
          bytes_[target] |= 4;
        }

        i += PlainFSA::kTransSize;
      }
    }

    PlainFSA fsa;
    fsa.num_trans_ = bytes_.size() / PlainFSA::kTransSize - active_path_.size() * 256;
    fsa.bytes_ = std::move(bytes_);

    return fsa;
  }

  PlainFSABuilder(const PlainFSABuilder&) = delete;
  PlainFSABuilder& operator=(const PlainFSABuilder&) = delete;

private:
  struct Range {
    size_t begin;
    size_t end;
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
  uint8_t get_symbol_(size_t trans) const {
    return bytes_[trans + 1];
  }
  size_t get_target_(size_t trans) const {
    size_t target = 0;
    std::memcpy(&target, &bytes_[trans + 2], PlainFSA::kAddrSize);
    return target;
  }
  void set_target_(size_t arc, size_t target) {
    std::memcpy(&bytes_[arc + 2], &target, PlainFSA::kAddrSize);
  }

  size_t get_lcp_(const std::string& str) const {
    const auto len = std::min(str.length(), active_len_);
    for (size_t i = 0; i < len; ++i) {
      const auto trans = active_path_[i].end - PlainFSA::kTransSize;
      if (static_cast<uint8_t>(str[i]) != get_symbol_(trans)) {
        return i;
      }
    }
    return len;
  }
  size_t freeze_state_(const Range& range) {
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

  size_t hash_(Range range) const {
    size_t h = 0;
    while (range.begin < range.end) {
      h = 17 * h + get_symbol_(range.begin);
      h = 17 * h + get_target_(range.begin);
      if (is_final_trans_(range.begin)) {
        h += 17;
      }
      range.begin += PlainFSA::kTransSize;
    }
    return h;
  }
  size_t serialize_(const Range& range) {
    const auto new_state = bytes_.size();
    auto len = range.end - range.begin;
    bytes_.resize(new_state + len);
    std::memcpy(&bytes_[new_state], &bytes_[range.begin], len);

    return new_state;
  }
  bool equivalent_(size_t lhs_begin, Range rhs) const {
    const auto len = rhs.end - rhs.begin;
    if (lhs_begin + len > bytes_.size()) {
      return false;
    }
    return std::memcmp(&bytes_[lhs_begin], &bytes_[rhs.begin], len) == 0;
  }

  size_t allocate_state_(size_t num_trans) {
    expand_buffers_();

    const auto new_state = bytes_.size();
    bytes_.resize(new_state + num_trans * PlainFSA::kTransSize);
    return new_state;
  }

  Range get_range_(size_t begin) const {
    Range range{begin, begin};
    while (!is_last_trans_(range.end)) {
      range.end += PlainFSA::kTransSize;
    }
    range.end += PlainFSA::kTransSize;
    return range;
  }

  void expand_buffers_() {
    if (bytes_.capacity() < bytes_.size() + kBufferGrowthSize) {
      bytes_.reserve(bytes_.capacity() + kBufferGrowthSize);
    }
  }
  void expand_active_path_(size_t len) {
    if (active_path_.size() < len) {
      const auto trans = active_path_.size();
      active_path_.resize(len);
      for (auto i = trans; i < len; ++i) {
        const auto state = allocate_state_(256);
        active_path_[i] = {state, state};
      }
    }
  }
  void expand_register_() {
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
};

}

#endif //ARRAY_FSA_FSABUILDER_HPP

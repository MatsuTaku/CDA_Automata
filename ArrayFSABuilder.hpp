//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_ARRAYFSABUILDER_HPP
#define ARRAY_FSA_ARRAYFSABUILDER_HPP

#include <unordered_map>

#include "ArrayFSA.hpp"
#include "PlainFSA.hpp"

namespace array_fsa {

class ArrayFSABuilder {
public:
  static constexpr size_t kAddrSize = 4;
  static constexpr size_t kElemSize = 1 + kAddrSize * 2;

  static ArrayFSA build(const PlainFSA& orig_fsa) {
    ArrayFSABuilder builder(orig_fsa);
    builder.build_();

    // Release
    ArrayFSA new_fsa;
    const auto num_elems = builder.bytes_.size() / kElemSize;

    while (num_elems >> (8 * ++new_fsa.next_size_ - 1));
    new_fsa.element_size_ = new_fsa.next_size_ + 1;

    new_fsa.bytes_.resize(num_elems * new_fsa.element_size_);

    for (size_t i = 0; i < num_elems; ++i) {
      const auto offset = i * kElemSize;
      const auto new_offset = i * new_fsa.element_size_;

      size_t next = builder.get_next_(offset) << 1 | (builder.is_final_(offset) ? 1 : 0);
      std::memcpy(&new_fsa.bytes_[new_offset], &next, new_fsa.next_size_);
      new_fsa.bytes_[new_offset + new_fsa.next_size_] = builder.get_check_(offset);

      if (builder.is_frozen_(offset)) {
        ++new_fsa.num_trans_;
      }
    }

    return new_fsa;
  }

  ArrayFSABuilder(const ArrayFSABuilder&) = delete;
  ArrayFSABuilder& operator=(const ArrayFSABuilder&) = delete;

private:
  static constexpr size_t kBlockSize = 256;
  static constexpr size_t kFreeBytes = 16 * kBlockSize * kElemSize; // like darts-clone

  const PlainFSA& orig_fsa_;

  std::vector<uint8_t> bytes_;
  std::unordered_map<size_t, size_t> state_map_;
  size_t unfrozen_head_ = 0;

  explicit ArrayFSABuilder(const PlainFSA& orig_fsa) : orig_fsa_(orig_fsa) {}
  ~ArrayFSABuilder() = default;

  void build_() {
    bytes_.reserve(static_cast<size_t>(orig_fsa_.get_num_trans() * 1.1 * kElemSize));

    expand_();
    freeze_state_(0);

    bytes_[0] |= 5; // is_final = is_used_next = true for terminal state

    arrange_(orig_fsa_.get_root_state(), 0);

    // reset
    state_map_ = std::unordered_map<size_t, size_t>();
  }

  void expand_() {
    const auto begin = bytes_.size();
    const auto end = begin + kBlockSize * kElemSize;

    bytes_.resize(end, 0);

    for (auto i = begin; i < end; i += kElemSize) {
      set_succ_(i, i + kElemSize);
      set_pred_(i, i - kElemSize);
    }

    if (unfrozen_head_ == 0) {
      // initial or full
      set_pred_(begin, end - kElemSize);
      set_succ_(end - kElemSize, begin);
      unfrozen_head_ = begin;
    } else {
      const auto unfrozen_tail = get_pred_(unfrozen_head_);
      set_pred_(begin, unfrozen_tail);
      set_succ_(end - kElemSize, unfrozen_head_);
      set_succ_(unfrozen_tail, begin);
      set_pred_(unfrozen_head_, end - kElemSize);
    }

    if (kFreeBytes <= begin) {
      close_block_(begin - kFreeBytes);
    }
  }

  void freeze_state_(size_t offset) {
    assert(!is_frozen_(offset));

    bytes_[offset] |= 2; // is_frozen = true

    const auto succ = get_succ_(offset);
    const auto pred = get_pred_(offset);

    // unlink
    set_succ_(pred, succ);
    set_pred_(succ, pred);

    std::memset(&bytes_[offset + 1], 0, kAddrSize * 2);

    if (offset == unfrozen_head_) {
      unfrozen_head_ = succ == offset ? 0 : succ;
    }
  }

  void close_block_(size_t begin) {
    const auto end = begin + kBlockSize * kElemSize;

    if (unfrozen_head_ != 0 && unfrozen_head_ < end) {
      for (auto i = begin; i < end; i += kElemSize) {
        if (!is_frozen_(i)) {
          freeze_state_(i);
          bytes_[i] &= ~2; // is_frozen = false
        }
      }
    }
  }

  void arrange_(size_t state, size_t offset) {
    const auto first_trans = orig_fsa_.get_first_trans(state);

    if (first_trans == 0) {
      set_next_(offset, 0); // to the terminal state
      return;
    }

    {
      auto it = state_map_.find(state);
      if (it != state_map_.end()) {
        // already visited state
        set_next_(offset, it->second);
        return;
      }
    }

    const auto next = find_next_(first_trans);
    if (offset_(next) >= bytes_.size()) {
      expand_();
    }

    set_next_(offset, next);
    state_map_.insert(std::make_pair(state, next));
    bytes_[offset_(next)] |= 4; // is_used_next = true

    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
      const auto symbol = orig_fsa_.get_trans_symbol(trans);
      const auto child_offset = offset_(next ^ symbol);

      freeze_state_(child_offset);
      set_check_(child_offset, symbol);

      if (orig_fsa_.is_final_trans(trans)) {
        bytes_[child_offset] |= 1; // is_final = true
      }
    }

    for (auto trans = first_trans; trans != 0; trans = orig_fsa_.get_next_trans(trans)) {
      const auto symbol = orig_fsa_.get_trans_symbol(trans);
      arrange_(orig_fsa_.get_target_state(trans), offset_(next ^ symbol));
    }
  }

  // so-called XCHECK
  size_t find_next_(size_t first_trans) const {
    const auto symbol = orig_fsa_.get_trans_symbol(first_trans);

    if (unfrozen_head_ != 0) {
      auto unfrozen_offset = unfrozen_head_;
      do {
        const auto next = index_(unfrozen_offset) ^symbol; // TODO: omit index_()
        if (check_next_(next, first_trans)) {
          return next;
        }
        unfrozen_offset = get_succ_(unfrozen_offset);
      } while (unfrozen_offset != unfrozen_head_);
    }

    return index_(bytes_.size()) ^ symbol;
  }

  bool check_next_(size_t next, size_t trans) const {
    if (is_used_next_(offset_(next))) {
      return false;
    }

    do {
      const auto index = next ^orig_fsa_.get_trans_symbol(trans);
      if (is_frozen_(offset_(index))) {
        return false;
      }
      trans = orig_fsa_.get_next_trans(trans);
    } while (trans != 0);

    return true;
  }

  // of array
  size_t index_(size_t offset) const {
    return offset / kElemSize;
  }
  // of codes
  size_t offset_(size_t index) const {
    return index * kElemSize;
  }

  // Getters
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

  // Setters
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
};

}

#endif //ARRAY_FSA_ARRAYFSABUILDER_HPP

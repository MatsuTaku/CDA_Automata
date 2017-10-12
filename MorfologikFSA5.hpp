//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_FSA5_HPP
#define ARRAY_FSA_FSA5_HPP

#include "basic.hpp"

namespace array_fsa {

class MorfologikFSA5 {
public:
  MorfologikFSA5() = default;
  ~MorfologikFSA5() = default;

  static std::string name() {
    return "MorfologikFSA5";
  }

  size_t get_root_state() const {
    auto epsilon_state = skip_trans_(get_first_trans(0));
    return get_destination_state_offset_(get_first_trans(epsilon_state));
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
    return get_destination_state_offset_(trans);
  }
  bool is_final_trans(size_t trans) const {
    return (bytes_[trans + 1] & 1) != 0;
  }

  size_t get_first_trans(size_t state) const {
    return node_data_length_ + state;
  }
  size_t get_next_trans(size_t trans) const {
    return is_last_trans(trans) ? 0 : skip_trans_(trans);
  }
  bool is_last_trans(size_t trans) const {
    return (bytes_[trans + 1] & 2) != 0;
  }
  uint8_t get_trans_symbol(size_t trans) const {
    return bytes_[trans];
  }

  size_t get_num_trans() const {
    size_t ret = 0;
    for (size_t i = 0; i < bytes_.size(); i = skip_trans_(i)) {
      ++ret;
    }
    return ret;
  }

  void read(std::istream& is) {
    if (0x5c667361 != morfologik_FileUtils::read_int(is)) {
      std::cerr << "Invalid file header magic bytes." << std::endl;
      return;
    }
    if (5 != read_val<uint8_t>(is)) {
      std::cerr << "Invalid version." << std::endl;
      return;
    }

    read_val<uint8_t>(is); // filler
    read_val<uint8_t>(is); // annotation
    gtl_ = read_val<uint8_t>(is);

    node_data_length_ = gtl_ >> 4 & 0x0F;
    gtl_ &= 0x0F;

    bytes_ = morfologik_FileUtils::fully_read(is);
  }

  size_t size_in_bytes() const {
    return 8 + bytes_.size();
  }

  void show_stat(std::ostream& os) const {
    using std::endl;
    os << "--- Stat of " << name() << " ---" << endl;
    os << "#trans: " << get_num_trans() << endl;
    os << "size:   " << size_in_bytes() << endl;
  }

  void print_for_debug(std::ostream& os) const {
    using std::endl;
    os << "\tLB\tF\tL\tN\tAD" << endl;

    size_t i = 0;
    while (i < bytes_.size()) {
      char c = get_trans_symbol(i);
      if (c == '\r') {
        c = '?';
      }
      os << i << "\t"
         << c << "\t"
         << bool_str(is_final_trans(i)) << "\t"
         << bool_str(is_last_trans(i)) << "\t"
         << bool_str(is_next_set_(i)) << "\t";
      if (!is_next_set_(i)) {
        os << get_goto_(i);
        i += 1 + gtl_;
      } else {
        i += 2;
      }
      os << endl;
    }
  }

  void swap(MorfologikFSA5& rhs) {
    bytes_.swap(rhs.bytes_);
    std::swap(node_data_length_, rhs.node_data_length_);
    std::swap(gtl_, rhs.gtl_);
  }

  MorfologikFSA5(const MorfologikFSA5&) = delete;
  MorfologikFSA5& operator=(const MorfologikFSA5&) = delete;

  MorfologikFSA5(MorfologikFSA5&& rhs) noexcept : MorfologikFSA5() {
    this->swap(rhs);
  }
  MorfologikFSA5& operator=(MorfologikFSA5&& rhs) noexcept {
    this->swap(rhs);
    return *this;
  }

private:
  std::vector<uint8_t> bytes_;
  size_t node_data_length_ = 0;
  size_t gtl_ = 0;

  size_t skip_trans_(size_t offset) const {
    return offset + (is_next_set_(offset) ? 2 : 1 + gtl_);
  }
  bool is_next_set_(size_t trans) const {
    return (bytes_[trans + 1] & 4) != 0;
  }
  size_t get_destination_state_offset_(size_t trans) const {
    return is_next_set_(trans) ? skip_trans_(trans) : get_goto_(trans);
  }
  size_t get_goto_(size_t trans) const {
    size_t ret = 0;
    std::memcpy(&ret, &bytes_[trans + 1], gtl_);
    return ret >> 3;
  }
};

}

#endif //ARRAY_FSA_FSA5_HPP

//
// Created by Kampersanda on 2017/05/26.
//

#ifndef ARRAY_FSA_CFSA2_HPP
#define ARRAY_FSA_CFSA2_HPP

#include "basic.hpp"

namespace csd_automata {
    
    class MorfologikCFSA2 {
    public:
        MorfologikCFSA2() = default;
        ~MorfologikCFSA2() = default;
        
        static std::string name() {
            return "MorfologikCFSA2";
        }
        
        bool isMember(const std::string &str) const {
            size_t state = get_root_state(), arc = 0;
            
            for (uint8_t c : str) {
                arc = get_trans(state, c);
                if (arc == 0) {
                    return false;
                }
                state = get_target_state(arc);
            }
            
            return is_final_trans(arc);
        }
        
        size_t get_root_state() const {
            return get_dest_state_offset_(get_first_trans(0));
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
            return get_dest_state_offset_(trans);
        }
        bool is_final_trans(size_t trans) const {
            return static_cast<bool>(bytes_[trans] & 0x20);
        }
        
        size_t get_first_trans(size_t state) const {
            return state;
        }
        size_t get_next_trans(size_t trans) const {
            return is_last_trans(trans) ? 0 : skip_trans_(trans);
        }
        bool is_last_trans(size_t trans) const {
            return static_cast<bool>(bytes_[trans] & 0x40);
        }
        uint8_t get_trans_symbol(size_t trans) const {
            auto index = bytes_[trans] & 0x1F;
            return index > 0 ? label_mapping_[index] : bytes_[trans + 1];
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
            
            if (-58 != read_val<int8_t>(is)) {
                std::cerr << "Invalid version." << std::endl;
                return;
            }
            
            morfologik_FileUtils::read_short(is); // flags
            
            label_mapping_.resize(read_val<uint8_t>(is));
            is.read(reinterpret_cast<char*>(&label_mapping_[0]), label_mapping_.size());
            
            bytes_ = morfologik_FileUtils::fully_read(is);
        }
        
        size_t size_in_bytes() const {
            return 4 + 1 + 2 + 1 + label_mapping_.size() + bytes_.size();
        }
        
        void ShowStatus(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << get_num_trans() << endl;
            os << "size:   " << size_in_bytes() << endl;
        }
        
        void PrintForDebug(std::ostream& os) const {
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
                    os << read_vint_(i + (static_cast<bool>(bytes_[i] & 0x1F) ? 1 : 2));
                }
                
                i = skip_trans_(i);
                os << endl;
            }
        }
        
        void swap(MorfologikCFSA2& rhs) {
            bytes_.swap(rhs.bytes_);
            label_mapping_.swap(rhs.label_mapping_);
        }
        
        MorfologikCFSA2(const MorfologikCFSA2&) = delete;
        MorfologikCFSA2& operator=(const MorfologikCFSA2&) = delete;
        
        MorfologikCFSA2(MorfologikCFSA2&& rhs) noexcept : MorfologikCFSA2() {
            this->swap(rhs);
        }
        MorfologikCFSA2& operator=(MorfologikCFSA2&& rhs) noexcept {
            this->swap(rhs);
            return *this;
        }
        
    private:
        std::vector<uint8_t> bytes_;
        std::vector<uint8_t> label_mapping_;
        
        size_t skip_trans_(size_t offset) const {
            auto flag = bytes_[offset++];
            if ((flag & 0x1F) == 0) {
                offset++;
            }
            if ((flag & 0x80) == 0) {
                offset = skip_vint_(offset);
            }
            return offset;
        }
        size_t skip_vint_(size_t offset) const {
            while (bytes_[offset++] & 0x80);
            return offset;
        }
        size_t read_vint_(size_t offset) const {
            auto b = bytes_[offset];
            auto value = static_cast<size_t>(b & 0x7F);
            for (size_t shift = 7; b & 0x80; shift += 7) {
                b = bytes_[++offset];
                value |= (b & 0x7F) << shift;
            }
            return value;
        }
        
        bool is_next_set_(size_t trans) const {
            return static_cast<bool>(bytes_[trans] & 0x80);
        }
        size_t get_dest_state_offset_(size_t trans) const {
            if (is_next_set_(trans)) {
                for (; !is_last_trans(trans); trans = get_next_trans(trans));
                return skip_trans_(trans);
            } else {
                return read_vint_(trans + (static_cast<bool>(bytes_[trans] & 0x1F) ? 1 : 2));
            }
        }
        
        friend class MorfologikCFSA2DictionaryFoundation;
        
    };
    
}

#endif //ARRAY_FSA_CFSA2_HPP

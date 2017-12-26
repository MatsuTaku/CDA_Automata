//
//  DArrayFSA.hpp
//  bench
//
//  Created by 松本拓真 on 2017/12/07.
//

#ifndef DArrayFSA_hpp
#define DArrayFSA_hpp

#include "DArrayFSAFrame.hpp"

#include "NextAccessory.hpp"
#include "CheckAccessory.hpp"
#include "FinalAccessory.hpp"
#include "LookupAccessory.hpp"

namespace double_array {
    
    // MARK: - object
    
    struct DArrayFSAAccessoryTypes {
        NextAccessoryType nextType;
        CheckAccessoryType checkType;
        FinalAccessoryType finalType;
    };
    
    
    // MARK: -
    
    class DArrayFSA : public DArrayFSAFrame {
    public:
        // MARK: constructor
        DArrayFSA() = default;
        ~DArrayFSA() = default;
        
        DArrayFSA(const DArrayFSA &) = delete;
        DArrayFSA & operator=(const DArrayFSA &) = delete;
        
        DArrayFSA(DArrayFSA &&rhs) noexcept : DArrayFSA() {
            this->swap(rhs);
        }
        
        DArrayFSA & operator=(DArrayFSA &&rhs) noexcept {
            this->swap(rhs);
            return *this;
        }
        
        // MARK: setup
        
        void setNextType(NextAccessoryType type) {
            next_type_ = type;
        }
        
        void setCheckType(CheckAccessoryType type) {
            check_type_ = type;
        }
        
        void setFinalType(FinalAccessoryType type) {
            final_type_ = type;
        }
        
        void setTypes(DArrayFSAAccessoryTypes types) {
            setNextType(types.nextType);
            setCheckType(types.checkType);
            setFinalType(types.finalType);
        }
        
        void initMembers();
        void generateName();
        
        // MARK: tool
        
        void show_stat(std::ostream &os);
        
        // MARK: file access
        
        void read(std::istream &is) override {
            DArrayFSAFrame::read(is);
            using array_fsa::read_val;
            next_type_ = read_val<NextAccessoryType>(is);
            check_type_ = read_val<CheckAccessoryType>(is);
            final_type_ = read_val<FinalAccessoryType>(is);
            initMembers();
            generateName();
        }
        
        void write(std::ostream &os) const override {
            DArrayFSAFrame::write(os);
            using array_fsa::write_val;
            write_val(next_type_, os);
            write_val(check_type_, os);
            write_val(final_type_, os);
        }
        
        void swap(DArrayFSA &rhs) {
            DArrayFSAFrame::swap(rhs);
            using std::swap;
            swap(next_type_, rhs.next_type_);
            swap(check_type_, rhs.check_type_);
            swap(final_type_, rhs.final_type_);
            initMembers();
            generateName();
        }
        
    private:
        NextAccessoryType next_type_ = NextAccessoryType::Plain;
        CheckAccessoryType check_type_ = CheckAccessoryType::NonLabel;
        FinalAccessoryType final_type_ = FinalAccessoryType::TopNext;
        
    };
    
    
    // MARK: - Inline function
    
    inline void DArrayFSA::initMembers() {
        NextAccessory nextAcs(next_type_);
        setNextAccessory(nextAcs);
        
        CheckAccessory checkAcs(check_type_);
        setCheckAccessory(checkAcs);
        switch (check_type_) {
            case CheckAccessoryType::NonLabel: {
                LookupAccessory lookupAcs(LookupAccessoryType::Plain);
                setLookupAccessory(lookupAcs);
            }
                break;
            case CheckAccessoryType::LabelPlain: {
                LookupAccessory lookupAcs(LookupAccessoryType::LabelDict);
                setLookupAccessory(lookupAcs);
            }
                break;
        }
        
        FinalAccessory finalAcs(final_type_);
        setFinalAccessory(finalAcs);
    }
    
    inline void DArrayFSA::generateName() {
        std::string name = "DArrayFSA";
        switch (next_type_) {
            case NextAccessoryType::Plain:
                name += "/Plain";
                break;
            case NextAccessoryType::DAC:
                name += "/DAC";
                break;
        }
        switch (check_type_) {
            case CheckAccessoryType::NonLabel:
                name += "/NonLabel";
                break;
            case CheckAccessoryType::LabelPlain:
                name += "/LabelPlain";
                break;
        }
        setName(name);
    }
    
    inline void DArrayFSA::show_stat(std::ostream &os) {
        using std::endl;
        os << "--- Stat of " << name() << " ---" << endl;
        os << "#trans: " << num_trans_ << endl;
        os << "#elems: " << getNumElements() << endl;
        os << "size:   " << sizeInBytes() << endl;
        os << "size bytes_:   " << array_fsa::size_vec(bytes_) << endl;
    }
    
}

#endif /* DArrayFSA_hpp */

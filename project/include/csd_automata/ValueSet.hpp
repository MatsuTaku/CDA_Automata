//
//  ValueSet.hpp
//  CSDAutomata
//
//  Created by 松本拓真 on 2018/10/09.
//

#ifndef ValueSet_hpp
#define ValueSet_hpp

#include "IOInterface.hpp"
#include "sim_ds/FitVector.hpp"

namespace csd_automata {

/** Store value set as 1 index.
 You can use favorite list data-structure while setting template parameter expricitly.
 */
class ValueSet : IOInterface {
public:
    using list_type = sim_ds::FitVector;
    
private:
    list_type values_;
    
public:
    // MARK: Constructors
    
    template <class Sequence>
    ValueSet(const Sequence& list) {
        set_list(list);
    }
    
    // MARK: Functionals
    
    size_t operator[](size_t index) const {
        if (index == 0) {
            std::cerr << "Error not support index of ValueSet[ " << index << std::endl;
            return -1;
        }
        return values_[index - 1];
    }
    
    size_t size() const {
        return values_.size();
    }
    
    template <class Sequence>
    void set_list(const Sequence& list) {
        values_ = list_type(list);
    }
    
    // MARK: IO
    
    void LoadFrom(std::istream& is) override {
        values_ = list_type(is);
    }
    
    void StoreTo(std::ostream& os) const override {
        values_.Write(os);
    }
    
    size_t size_in_bytes() const override {
        return values_.size_in_bytes();
    }
    
    // MARK: Copy guard
    
    ValueSet() = default;
    ~ValueSet() = default;
    
    ValueSet(const ValueSet&) = delete;
    ValueSet& operator=(const ValueSet&) = delete;
    
    ValueSet(ValueSet&&) noexcept = default;
    ValueSet& operator=(ValueSet&&) noexcept = default;
    
    
};

}

#endif /* ValueSet_hpp */

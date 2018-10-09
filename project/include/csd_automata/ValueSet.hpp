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
        
        // MARK: Constructors
        
        ValueSet() = default;
        
        template <class CONTAINER>
        ValueSet(const CONTAINER& list) {
            setList(list);
        }
        
        ~ValueSet() = default;
        
        // MARK: Copy guard
        
        ValueSet(const ValueSet&) = delete;
        ValueSet& operator=(const ValueSet&) = delete;
        
        ValueSet(ValueSet&&) noexcept = default;
        ValueSet& operator=(ValueSet&&) noexcept = default;
        
        // MARK: - Functionals
        
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
        
        template <class CONTAINER>
        void setList(const CONTAINER& list) {
            values_ = list_type(list);
        }
        
        // MARK: IO
        
        void read(std::istream& is) override {
            values_ = list_type(is);
        }
        
        void write(std::ostream& os) const override {
            values_.write(os);
        }
        
        size_t sizeInBytes() const override {
            return values_.sizeInBytes();
        }
        
    private:
        list_type values_;
        
    };
    
}

#endif /* ValueSet_hpp */

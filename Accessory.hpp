//
//  Accessory.hpp
//  double_array
//
//  Created by 松本拓真 on 2017/12/06.
//

#ifndef Accessory_hpp
#define Accessory_hpp

namespace double_array {
    
    class DArrayFSAFrame;
    
    template <typename T>
    class Accessory {
    protected:
        using getterType = std::function<T(const DArrayFSAFrame&, size_t)>;
        using setterType = std::function<void(DArrayFSAFrame&, size_t, T)>;
        
    public:
        void initGetterSetter(getterType getter, setterType setter) {
            getter_ = getter;
            setter_ = setter;
        }
        
        T get(const DArrayFSAFrame &fsa, size_t index) const {
            return getter_(fsa, index);
        }
        
        void set(DArrayFSAFrame &fsa, size_t index, T value) const {
            setter_(fsa, index, value);
        }
        
    private:
        getterType getter_;
        setterType setter_;
        
    };
    
    
    class Scout {
    protected:
        using type = std::function<bool(const DArrayFSAFrame&, const std::string&)>;
        
    public:
        void initScouts(type scout) {
            scout_ = scout;
        }
        
        bool scouts(const DArrayFSAFrame &fsa, const std::string &str) const {
            return scout_(fsa, str);
        }
        
    private:
        type scout_;
    };
    
}


#endif /* Accessory_hpp */

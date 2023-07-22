//
//  ParameterInterpolator.h
//  NewProject - Shared Code
//
//  Created by Eugene Clewlow on 7/21/23.
//

#ifndef ParameterInterpolator_h
#define ParameterInterpolator_h

class ParameterInterpolator {
public:
    ParameterInterpolator() { }
    ~ParameterInterpolator() {
    }

    
    inline void Update(float current, float target, size_t size) {
        if(target_ != target) {
            target_ = target;
            tick_ = 0;
            increment_ = (target - current) / size;
            size_ = size;
        }
        else if(current == target) {
            increment_ = 0;
        } else {
            target_ = target;
            tick_ = 0;
            increment_ = (target - current) / size;
            size_ = size;
        }
    }
    
    inline float Next() {
        tick_++;
        return increment_;
    }

    
private:
    float* state_;
    float value_;
    float increment_;
    float target_;
    size_t size_;
    uint32_t tick_;
};


#endif /* ParameterInterpolator_h */

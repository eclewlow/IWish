//
//  PitchEnvelope.h
//  NewProject - Shared Code
//
//  Created by Eugene Clewlow on 7/24/23.
//

#ifndef PitchEnvelope_h
#define PitchEnvelope_h

#include <numbers>
#include <cmath>
#include "SynthParams.h"
#import "ParameterInterpolator.h"


class PitchEnvelope {
public:
    PitchEnvelope(SynthParams* synthParams={}, double sampleRate = 44100.0) {
        mSampleRate = sampleRate;
        mSynthParams = synthParams;
        phase = 0.0;
        phase_increment = 2 / sampleRate;
        
        delay_interpolator = ParameterInterpolator();
        attack_interpolator = ParameterInterpolator();
        curve_interpolator = ParameterInterpolator();

        delay_ = 0;
        attack_ = 0;
        curve_ = 0;

    }
    
    float exp_func(float x, float d, float z, float c) {
        float value;
        value = 1 - (pow(c, 1 + z * (d - x)) - 1) / (c-1);
        if(value > 1.0) {
            value = 1.0;
        }
        if(value < 0) {
            value = 0.0f;
        }
        return value;
    }
    
    static inline double clamp(double input, double low, double high) {
        return std::min(std::max(input, low), high);
    }

    double process() {
        delay_interpolator.Update(delay_, mSynthParams->pitch_envelope_delay, 24*12);
        attack_interpolator.Update(attack_, pow(10, mSynthParams->pitch_envelope_attack), 24*12);
        curve_interpolator.Update(curve_, mSynthParams->pitch_envelope_curve, 24*12);

        delay_ += delay_interpolator.Next();
        attack_ += attack_interpolator.Next();
        curve_ += curve_interpolator.Next();

        
        phase += phase_increment;
        
        if(phase >= 1.0f) {
            phase = 1.0f;
            mGain = 1.0;
        } else {
            // x is 0 to 1.
            mGain = exp_func(phase, delay_, attack_, curve_);
        }
        
        mGain = clamp(mGain, 0.0f, 1.0f);

        return mGain;
    }
    
    void noteOn() {
        phase = 0;
    }
    
    void noteOff() {
//        phase = 0;
    }
private:
    double mGain = {0.0};
    double mSampleRate = { 0.0 };
    float phase;
    float phase_increment;
    
    float delay_;
    float attack_;
    float curve_;
    ParameterInterpolator delay_interpolator;
    ParameterInterpolator attack_interpolator;
    ParameterInterpolator curve_interpolator;

    SynthParams *mSynthParams;
};



#endif /* PitchEnvelope_h */

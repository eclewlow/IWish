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


class PitchEnvelope {
public:
    PitchEnvelope(SynthParams* synthParams={}, double sampleRate = 44100.0) {
        mSampleRate = sampleRate;
        mSynthParams = synthParams;
        phase = 0.0;
        phase_increment = 2 / sampleRate;
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
        float delay, attack, curve;
        
        delay  = mSynthParams->pitch_envelope_delay;
        attack = pow(10, mSynthParams->pitch_envelope_attack);
        curve  = mSynthParams->pitch_envelope_curve;
        
        phase += phase_increment;
        
        if(phase >= 1.0f) {
            phase = 1.0f;
            mGain = 1.0;
        } else {
            // x is 0 to 1.
            mGain = exp_func(phase, delay, attack, curve);
        }
        
        mGain = clamp(mGain, 0.0f, 1.0f);

        return mGain;
    }
    
    void noteOn() {
    }
    
    void noteOff() {
//        phase = 0;
    }
private:
    double mGain = {0.0};
    double mSampleRate = { 0.0 };
    float phase;
    float phase_increment;
    
    SynthParams *mSynthParams;
};



#endif /* PitchEnvelope_h */

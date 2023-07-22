//
//  ADSREnvelope.h
//  InstrumentExtension
//
//  Created by Eugene Clewlow on 1/25/23.
//

#pragma once

#include <numbers>
#include <cmath>
#include "SynthParams.h"

class ADSREnvelope {
public:
    typedef enum {
        kOff = 0,
        kAttack =1,
        kDecay =2,
        kSustain =3,
        kRelease =4
    } State;
    
    typedef enum {
        kLinearReleaseMode = 0,
        kExponentialReleaseMode = 1
    } ReleaseMode;

    typedef enum {
        kLinearAttackMode = 0,
        kLogarithmicAttackMode = 1
    } AttackMode;

    typedef enum {
        ADSR_TYPE_VCA = 0,
        ADSR_TYPE_VCF = 1,
    } ADSRType;

    typedef enum {
        kEnvelopeRetriggerMode = 0,
        kLegatoMode = 1
    } LegatoMode;

    
    ADSREnvelope(ADSRType adsrType = ADSR_TYPE_VCA, SynthParams* synthParams={}, double sampleRate = 44100.0) {
        mSampleRate = sampleRate;
        mState = kOff;
        mADSRType = adsrType;
        mSynthParams = synthParams;
    }
    
    double process() {
        float attack, decay, sustain, release;
        
        if(mADSRType == ADSR_TYPE_VCA) {
            attack = mSynthParams->vca_attack;
            decay = mSynthParams->vca_decay;
            sustain = mSynthParams->vca_sustain;
            release = mSynthParams->vca_release;
        } else if(mADSRType == ADSR_TYPE_VCF) {
            attack = mSynthParams->vcf_attack;
            decay = mSynthParams->vcf_decay;
            sustain = mSynthParams->vcf_sustain;
            release = mSynthParams->vcf_release;
        }
        
        if(mState == kOff) {
            return 0.0f;
        }
        else if(mState == kAttack)
        {
            mGain = (mCoeff * mGain) + ((1.0 - mCoeff) * 1.0);

            if(mGain >= 1.0 - 0.001) {
                mGain = 1.0;
                mState = kDecay;
                double timeConstant = (decay) * mSampleRate * 0.001;
                mCoeff = pow(1.0/0.001, -1.0/timeConstant);
                return mGain;
            } else {
                return mGain;
            }
            
        } else if(mState == kDecay) {
            
            mGain = (mCoeff * mGain) + ((1.0 - mCoeff) * sustain);
            
            if(mGain <= sustain + 0.001) {
                mGain = sustain;
                mState = kSustain;
                mCoeff = 0.0;
                return mGain;
            } else {
                return mGain;
            }

        } else if(mState == kSustain) {
            mGain = sustain;
            return mGain;
        } else if(mState == kRelease) {

            mGain = (mCoeff * mGain) + ((1.0 - mCoeff) * 0.0);
            
            if(mGain <= 0.0 + 0.001) {
                mGain = 0.0;
                mState = kOff;
                mCoeff = 0.0;
                return mGain;
            } else {
                return mGain;
            }

        }
        
        return 0.0f;
    }
    
#define TIME_CONSTANT 0.001
    
    void noteOn() {
        mState = kAttack;

        if(mADSRType == ADSR_TYPE_VCA) {
            double timeConstant = (mSynthParams->vca_attack) * mSampleRate * TIME_CONSTANT;
            mCoeff = pow(1.0/TIME_CONSTANT, -1.0/timeConstant);
        } else if(mADSRType == ADSR_TYPE_VCF) {
            double timeConstant = (mSynthParams->vcf_attack) * mSampleRate * 0.001;
            mCoeff = pow(1.0/0.001, -1.0/timeConstant);
        }

        if(mLegatoMode == kEnvelopeRetriggerMode) {
            mGain = 0.0;
        }
    }
    
    void noteOff() {
        mState = kRelease;
        if(mADSRType == ADSR_TYPE_VCA) {
            double timeConstant = (mSynthParams->vca_release) * mSampleRate * TIME_CONSTANT;
            mCoeff = pow(1.0/TIME_CONSTANT, -1.0/timeConstant);
        } else if(mADSRType == ADSR_TYPE_VCF) {
            double timeConstant = (mSynthParams->vcf_release) * mSampleRate * 0.001;
            mCoeff = pow(1.0/0.001, -1.0/timeConstant);
        }
    }
    
    double getEnvelopeGain() {
        if(mState == kOff) {
            return 0.0f;
        }

        return mGain;
    }
    
    State getEnvelopeState() const {
        return mState;
    }

private:
    State mState = kOff;
    ReleaseMode mReleaseMode = kExponentialReleaseMode;
    LegatoMode mLegatoMode = kLegatoMode;
    ADSRType mADSRType = ADSR_TYPE_VCA;
    AttackMode mAttackMode = kLogarithmicAttackMode;
    
    double mGain = {0.0};
    double mCoeff = {0.0};
    double mSampleRate = { 0.0 };
    SynthParams *mSynthParams;
};

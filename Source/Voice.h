//
//  Voice.h
//  NewProject - Shared Code
//
//  Created by Eugene Clewlow on 7/16/23.
//

#ifndef Voice_h
#define Voice_h

#include "SynthParams.h"

class Voice {
public:
    Voice(double sampleRate = 44100.0, SynthParams* synthParams={}){
//        double sampleRate = 44100.0, SynthParams* synthParams={}):mVCAEnv(ADSREnvelope::ADSR_TYPE_VCA, synthParams),mVCFEnv(ADSREnvelope::ADSR_TYPE_VCF, synthParams), mResonantFilter(sampleRate, synthParams),mResonantFilter2(sampleRate, synthParams), svf(sampleRate, synthParams) {
//        mSampleRate = sampleRate;
//        mOsc1 =Oscillator(sampleRate, synthParams);
//        mOsc2 =Oscillator(sampleRate, synthParams);
//        mSynthParams = synthParams;
//        fm_frequency = ParameterInterpolator();
//        svf.Init();
    }
    
//    inline double Oscillator2MIDINoteToFrequency(double note) {
//        constexpr auto kMiddleA = 440.0;
//        double fine_tune = mSynthParams->fine_tune/100.0f;
//        double coarse_tune = mSynthParams->coarse_tune/100.0f;
//        double pitchBend = (mSynthParams->pitch_bend - 0x40) * 12.0 / 0x40;
//
//        // pitch bend is 0x00 -> 0x40 -> 0x7F
//        // this allows for 64 values below middle
//        // and allows for 63 values above middle,
//        // including the middle this totals to 128 possible values
//        return (kMiddleA / 32.0) * pow(2, (((note+fine_tune+coarse_tune+pitchBend) - 9.0) / 12.0));
//    }
//
//    inline double Oscillator1MIDINoteToFrequency(double note) {
//        constexpr auto kMiddleA = 440.0;
//        double pitchBend = (mSynthParams->pitch_bend - 0x40) * 12.0 / 0x40;
//
//        return (kMiddleA / 32.0) * pow(2, (((note+pitchBend) - 9.0) / 12.0));
//    }
    
    bool isFinished() const {
//        return mVCAEnv.getEnvelopeState() == ADSREnvelope::kOff;
        return !mNoteOn;
    }
    
    double process() {
        
        /* do FM manipulation here.
         the modulator should calculate normally.
         
         the output of the modulator
         should be sent to the 2nd oscillator and should be added onto its phase
         */
//        float oscillatorOutput;
//        if(mSynthParams->oscillator_mode == OSCILLATOR_MODE_FM) {
//
//            // add morph 0.5 feedback
//            // phase feedback is 0 cancel the next step.
//            // modulator_freq * (1+feedback * feedback * 0.5 * previous sample)
//            // modulator phase actually gets incremented
//            // then modulator feedback, phase temporarily incfemented by 0.25 * feedback *feedback * previous_sample
//
//            fm_frequency.Update(fm_gain_, mSynthParams->fm_gain, 24);
//            fm_feedback.Update(fm_feedback_, mSynthParams->fm_feedback, 24);
//
//            fm_gain_ += fm_frequency.Next();
//            fm_feedback_ += fm_feedback.Next();
//            float feedback = fm_feedback_;
//
//            float modulator_fb = feedback > 0.0f ? 0.25f * feedback * feedback : 0.0f;
//            float phase_feedback = feedback < 0.0f ? 0.5f * feedback * feedback : 0.0f;
//
//            oscillatorOutput = mOsc1.process((previous_sample_ * phase_feedback) + (previous_sample_ * modulator_fb));
//            oscillatorOutput = mOsc2.process(oscillatorOutput * fm_gain_);
//
//            previous_sample_ += 0.05 * (oscillatorOutput - previous_sample_);
//
//        } else {
//
//            fm_frequency.Update(fm_gain_, mSynthParams->fm_gain, 24);
//            fm_feedback.Update(fm_feedback_, mSynthParams->fm_feedback, 24);
//
//            fm_gain_ += fm_frequency.Next();
//            fm_feedback_ += fm_feedback.Next();
//            float feedback = fm_feedback_;
//
//            float modulator_fb = feedback > 0.0f ? 0.25f * feedback * feedback : 0.0f;
//            float phase_feedback = feedback < 0.0f ? 0.5f * feedback * feedback : 0.0f;
//
//
//            float syncin;
//            float syncout;
//
//            syncin = -1.0;
//            syncout = -1.0;
//
//            oscillatorOutput = mOsc1.process((previous_sample_ * phase_feedback) + (previous_sample_ * modulator_fb), &syncin, &syncout);
//
//            syncin = syncout;
//            syncout = -1.0;
//
//            if(fm_gain_ > 0.0)
//                oscillatorOutput = mOsc2.process(oscillatorOutput * fm_gain_, &syncin, &syncout);
//            else
//                oscillatorOutput += mOsc2.process(0.0, &syncin, &syncout);
//
//            previous_sample_ += 0.05 * (oscillatorOutput - previous_sample_);
//        }
//
//        float vcfEnvelopeControlVoltage = mVCFEnv.process();
//
//        float filterState2Output = svf.Process<FILTER_MODE_LOW_PASS>(oscillatorOutput, vcfEnvelopeControlVoltage, mNote);
//
//        float vcaEnvelopeOutput = mVCAEnv.process() * filterState2Output;
//
//        return vcaEnvelopeOutput;
    }
    
    void noteOn(int note) {
        mNote = note;
        mNoteOn = true;
//        if (mSynthParams->oscillator_mode == OSCILLATOR_MODE_FM) {
//            double osc1freq = Oscillator1MIDINoteToFrequency(mNote);
//            double osc2freq = Oscillator2MIDINoteToFrequency(mNote);
//            mOsc1.setFrequency(osc1freq * (mSynthParams->fm_gain == 0.0 ? 1.0: mSynthParams->fm_ratio));
//            mOsc2.setFrequency(osc2freq);
//        } else {
//            double osc1freq = Oscillator1MIDINoteToFrequency(mNote);
//            double osc2freq = Oscillator2MIDINoteToFrequency(mNote);
//            mOsc1.setFrequency(osc1freq * (mSynthParams->fm_gain == 0.0 ? 1.0: mSynthParams->fm_ratio));
//            mOsc2.setFrequency(osc2freq);
//        }
//        mVCAEnv.noteOn();
//        mVCFEnv.noteOn();
    }
    
    void noteOff(int note) {
//        mVCAEnv.noteOff();
//        mVCFEnv.noteOff();
        mNoteOn = false;
    }
    
    void recomputeFrequency() {
//        if (mSynthParams->oscillator_mode == OSCILLATOR_MODE_FM) {
//            double osc1freq = Oscillator1MIDINoteToFrequency(mNote);
//            double osc2freq = Oscillator2MIDINoteToFrequency(mNote);
//            mOsc1.setFrequency(osc1freq * (mSynthParams->fm_gain == 0.0 ? 1.0: mSynthParams->fm_ratio));
//            mOsc2.setFrequency(osc2freq);
//        } else {
//            double osc1freq = Oscillator1MIDINoteToFrequency(mNote);
//            double osc2freq = Oscillator2MIDINoteToFrequency(mNote);
//            mOsc1.setFrequency(osc1freq * (mSynthParams->fm_gain == 0.0 ? 1.0: mSynthParams->fm_ratio));
//            mOsc2.setFrequency(osc2freq);
//        }
    }
    
    void reset() {
//        mOsc1.reset();
//        mOsc2.reset();
    }
    
    int getNote() const {
        return mNote;
    }
    
    void setNote(int note) {
        mNote = note;
    }
    
private:
    double mOmega = { 0.0 };
    double mDeltaOmega = { 0.0 };
    double mSampleRate = { 0.0 };
    int mNote;
    double mFrequency = {0.0f};
    float previous_sample_ = 0.0;
    
    bool mNoteOn=false;
    
//
//    float fm_gain_ = 0.0;
//    float fm_feedback_ = 0.0;
//
//    ParameterInterpolator fm_frequency;
//    ParameterInterpolator fm_feedback;
//
//    Oscillator mOsc1;
//    Oscillator mOsc2;
//    ADSREnvelope mVCAEnv;
//    ADSREnvelope mVCFEnv;
//    BiquadFilter mResonantFilter;
//    BiquadFilter mResonantFilter2;
//    Svf svf;
//    SynthParams* mSynthParams;
};

#endif /* Voice_h */

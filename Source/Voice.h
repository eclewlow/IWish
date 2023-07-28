//
//  Voice.h
//  NewProject - Shared Code
//
//  Created by Eugene Clewlow on 7/16/23.
//

#ifndef Voice_h
#define Voice_h

#include "SynthParams.h"
#import "ADSREnvelope.h"
#import "ParameterInterpolator.h"
#import "PitchEnvelope.h"
#import "Filter.h"


class Voice {
public:
    Voice(double sampleRate = 44100.0, SynthParams* synthParams={}):mVCAEnv(ADSREnvelope::ADSR_TYPE_VCA, synthParams, sampleRate),mPitchEnv(synthParams, sampleRate), svf(sampleRate, synthParams), audioBuffer(){
        //        double sampleRate = 44100.0, SynthParams* synthParams={}):mVCAEnv(ADSREnvelope::ADSR_TYPE_VCA, synthParams),mVCFEnv(ADSREnvelope::ADSR_TYPE_VCF, synthParams), mResonantFilter(sampleRate, synthParams),mResonantFilter2(sampleRate, synthParams), svf(sampleRate, synthParams) {
        mSynthParams = synthParams;
        mSampleRate = sampleRate;
        mSampleRate = sampleRate;
        nyquist = 0.5 * sampleRate;
        inverseNyquist = 1.0 / nyquist;
        //        audioBuffer = new juce::AudioBuffer<float>();//1, 2048);
        resetAudioBuffer(1, 0);


        m1_phase_increment_interpolator = ParameterInterpolator();
        s1_phase_increment_interpolator = ParameterInterpolator();

        m1_phase = 0;
        s1_phase = 0;
        m1_phase_increment = 0;
        s1_phase_increment = 0;
        
        pitch_interpolator = ParameterInterpolator();
        formant_interpolator = ParameterInterpolator();
        
        pitch_ = 0;
        formant_ = 0;

        pitch_shift_interpolator = ParameterInterpolator();
        formant_shift_interpolator = ParameterInterpolator();

        pitch_shift_ = 0;
        formant_shift_ = 0;
        
        svf.Init();
    }
    
    bool isFinished() const {
        return mVCAEnv.getEnvelopeState() == ADSREnvelope::kOff;
        //        return !mNoteOn;
    }
    
    static inline double clamp(double input, double low, double high) {
        return std::min(std::max(input, low), high);
    }

    double process(float input) {
        double sample = 0.0f;
        
        //        if(isAnyNoteOn()) {
        
        
//        test_noxfade_overwrite_buffer(&sample, input);
//        test_xfade_japan(&sample, input);
        float pitchEnvelope = mPitchEnv.process();
        
        test_xfade_japan_master_phase_increment(&sample, input, pitchEnvelope);

        double cutoff = clamp(8000.0f * inverseNyquist, 0.0005444f, 0.9070295f);
        double resonance = clamp(0.10, 0.10f, 100.0f);

        svf.set_f_q<FREQUENCY_EXACT>(cutoff, resonance);
        float filtered_output = svf.Process<FILTER_MODE_LOW_PASS>((float)sample);

        float vcaEnvelopeOutput = mVCAEnv.process() * filtered_output;
        
        return vcaEnvelopeOutput;
    }
    
    void noteOn(int note) {
        if(note != mNote) {
            mNote = note;
            
            float shifted_note = note - 24.0f - 24.0f;
            resetAudioBuffer(1, ceil(mSampleRate / midiNoteToFrequency(shifted_note)));
        }
        
        mVCAEnv.noteOn();
        mPitchEnv.noteOn();
        m1_phase = 0;
        s1_phase = 0;
        //        mVCFEnv.noteOn();
    }
    
    void noteOff(int note) {
        mVCAEnv.noteOff();
        mPitchEnv.noteOff();
        //        mVCFEnv.noteOff();
        //        mNoteOn = false;
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
    
    
    
    inline double getInterpolatedSample(double t) {
        int integral = static_cast<int>(t);
        float fractional = t - static_cast<float>(integral);
        
        float s0 = audioBuffer.getSample(0, integral);
        float s1 = audioBuffer.getSample(0, integral + 1);
        
        float f = s0 + (s1 - s0) * fractional;
        return f;
    }
    
    inline double midiNoteToFrequency(double note) {
        constexpr auto kMiddleA = 440.0;
        //        double pitchBend = (mSynthParams->pitch_bend - 0x40) * 12.0 / 0x40;
        double pitchBend = 0;
        //        note = clamp(note, 0.0, 127.0);
        return (kMiddleA / 32.0) * pow(2, (((note+pitchBend) - 9.0) / 12.0));
    }
    
    
    
    void resetAudioBuffer(int channel, int size) {
        audioBuffer.clear();
        audioBuffer.setSize(channel, size*2);
        writeSampleIndex = 0;
        xfadeSampleIndex = 0;
    }
    
    void writeAudioBuffer(float data) {
        if(writeSampleIndex < audioBuffer.getNumSamples()) {
            audioBuffer.setSample(0, writeSampleIndex++, data);
        }
    }
        
    void test_xfade_japan_master_phase_increment(double* sample, float input, float pitchEnvelope) {
        //        resetAudioBuffer(1, ceil(mSampleRate / midiNoteToFrequency(12)));
        //        audioBuffer.clear();
        //        audioBuffer.setSize(1, ceil(mSampleRate / midiNoteToFrequency(12))*2);
        //        audioBuffer.setSample(0, 0, 1);
        //        writeSampleIndex = 0;
        //        xfadeSampleIndex = 0;
        
        if(audioBuffer.getNumSamples() == 0)
        {
            *sample = input;
            return;
        }
        
        writeAudioBuffer(input);
        
        
        pitch_interpolator.Update(pitch_, mSynthParams->pitch, 24*12);
        formant_interpolator.Update(formant_, mSynthParams->formant, 24*12);
        
        pitch_ += pitch_interpolator.Next();
        formant_ += formant_interpolator.Next();
        

        pitch_shift_interpolator.Update(pitch_shift_, mSynthParams->pitch_envelope_amount, 24*12);
        formant_shift_interpolator.Update(formant_shift_, mSynthParams->formant_envelope_amount, 24*12);

        pitch_shift_ += pitch_shift_interpolator.Next();
        formant_shift_ += formant_shift_interpolator.Next();
        
        float pitch = mNote + (pitch_ * 48.0f - 24.0f) + (pitch_shift_ * 24.0f - 12.0f) * pitchEnvelope;
        float formant = mNote + 48 * (formant_ + pitch_ - 1) + 24.0f * (formant_shift_ + pitch_shift_ - 1) * pitchEnvelope;
        
        
        m1_phase_increment_interpolator.Update(m1_phase_increment, midiNoteToFrequency(pitch) / mSampleRate, 24*12);
        s1_phase_increment_interpolator.Update(s1_phase_increment, midiNoteToFrequency(formant) / mSampleRate, 24*12);

        m1_phase_increment += m1_phase_increment_interpolator.Next();
        s1_phase_increment += s1_phase_increment_interpolator.Next();

//        m1_phase_increment = midiNoteToFrequency(pitch) / mSampleRate;
//        s1_phase_increment = midiNoteToFrequency(formant) / mSampleRate;
        
        if (writeSampleIndex > 1 / m1_phase_increment)
        {
            m1_phase += m1_phase_increment;
            s1_phase += s1_phase_increment;
            
            if(m1_phase >= 1.0 + mSynthParams->xfade) {
                m1_phase -= 1.0;
                float t = m1_phase / m1_phase_increment;
                s1_phase = t * s1_phase_increment;
                s1_phase = fmod(s1_phase, 2.0);
            }
            
            if(m1_phase >= 1.0 && mSynthParams->xfade > 0.0) {
                float m2_phase = m1_phase - 1.0;
                float t = m2_phase / m1_phase_increment;
                float s2_phase = t * s1_phase_increment;
                s2_phase = fmod(s2_phase, 2.0);
                
                float output1;
                float output2;
                // we are in a fade region of two masters overlapping.
                
                if(s1_phase >= 1.0 + mSynthParams->xfade) {
                    s1_phase -= 1.0;
                }
                if(s1_phase >= 1.0 && mSynthParams->xfade > 0.0) {
                    // we are in a region of two formants overlapping on itself
                    // cross fade m1's s1 with itself
                    float sn_phase = s1_phase - 1.0;
                    
                    float t1 = s1_phase / m1_phase_increment;
                    float tn = sn_phase / m1_phase_increment;
                    
                    float s1_output = getInterpolatedSample(t1);
                    float sn_output = getInterpolatedSample(tn);
                    
                    float fade_sn = sn_phase / mSynthParams->xfade;
                    output1 = fade_sn * sn_output + (1-fade_sn) * s1_output;
                    // do formant mix
                } else {
                    // no formant mix
                    float t1 = s1_phase / m1_phase_increment;
                    
                    output1 = getInterpolatedSample(t1);
                }
                
                if(s2_phase >= 1.0 + mSynthParams->xfade) {
                    s2_phase -= 1.0;
                }
                if(s2_phase >= 1.0 && mSynthParams->xfade > 0.0) {
                    // we are in a region of two formants overlapping
                    // cross fade m2's s2 with itself
                    float sn_phase = s2_phase - 1.0;
                    
                    float t2 = s2_phase / m1_phase_increment;
                    float tn = sn_phase / m1_phase_increment;
                    
                    float s2_output = getInterpolatedSample(t2);
                    float sn_output = getInterpolatedSample(tn);
                    
                    float fade_sn = sn_phase / mSynthParams->xfade;
                    output2 = fade_sn * sn_output + (1-fade_sn) * s2_output;
                    // do formant mix
                } else {
                    // no formant mix
                    float t2 = s2_phase / m1_phase_increment;
                    
                    output2 = getInterpolatedSample(t2);
                }
                
                float fade_m2 = m2_phase / mSynthParams->xfade;
                *sample = fade_m2 * output2 + (1-fade_m2) * output1;
                
            } else {
                // no master cross fade
                
                if(s1_phase >= 1.0 + mSynthParams->xfade) {
                    s1_phase -= 1.0;
                }
                if(s1_phase >= 1.0 && mSynthParams->xfade > 0.0) {
                    // we are in a region of two formants overlapping on itself
                    // cross fade m1's s1 with itself
                    float sn_phase = s1_phase - 1.0;
                    
                    float t1 = s1_phase / m1_phase_increment;
                    float tn = sn_phase / m1_phase_increment;
                    
                    float s1_output = getInterpolatedSample(t1);
                    float sn_output = getInterpolatedSample(tn);
                    
                    float fade_sn = sn_phase / mSynthParams->xfade;
                    *sample = fade_sn * sn_output + (1-fade_sn) * s1_output;
                    // do formant mix
                } else {
                    // no formant mix
                    float t1 = s1_phase / m1_phase_increment;
                    
                    *sample = getInterpolatedSample(t1);
                }
            }
        } else {
            *sample = input;
        }
    }
    
    double getEnvelopeGain() {
        return mVCAEnv.getEnvelopeGain();
    }
    
private:
    double mOmega = { 0.0 };
    double mDeltaOmega = { 0.0 };
    double nyquist;
    double inverseNyquist;
    double mSampleRate = { 0.0 };
    int mNote;
    double mFrequency = {0.0f};
    
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
    ADSREnvelope mVCAEnv;
    PitchEnvelope mPitchEnv;
    //    ADSREnvelope mVCFEnv;
    //    BiquadFilter mResonantFilter;
    //    BiquadFilter mResonantFilter2;
    //    Svf svf;
    juce::AudioBuffer<float> audioBuffer;
    uint writeSampleIndex;
    uint xfadeSampleIndex;
    
    
    SynthParams* mSynthParams;
    double m1_phase;
    double s1_phase;
    
    double m1_phase_increment;
    double s1_phase_increment;
    
    ParameterInterpolator m1_phase_increment_interpolator;
    ParameterInterpolator s1_phase_increment_interpolator;
    
    ParameterInterpolator pitch_interpolator;
    ParameterInterpolator formant_interpolator;

    double pitch_;
    double formant_;

    ParameterInterpolator pitch_shift_interpolator;
    ParameterInterpolator formant_shift_interpolator;

    double pitch_shift_;
    double formant_shift_;
    
    Svf svf;

};

#endif /* Voice_h */

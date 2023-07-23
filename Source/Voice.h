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


class Voice {
public:
    Voice(double sampleRate = 44100.0, SynthParams* synthParams={}):mVCAEnv(ADSREnvelope::ADSR_TYPE_VCA, synthParams, sampleRate),audioBuffer(){
        //        double sampleRate = 44100.0, SynthParams* synthParams={}):mVCAEnv(ADSREnvelope::ADSR_TYPE_VCA, synthParams),mVCFEnv(ADSREnvelope::ADSR_TYPE_VCF, synthParams), mResonantFilter(sampleRate, synthParams),mResonantFilter2(sampleRate, synthParams), svf(sampleRate, synthParams) {
        mSynthParams = synthParams;
        mSampleRate = sampleRate;
        //        audioBuffer = new juce::AudioBuffer<float>();//1, 2048);
        resetAudioBuffer(1, 0);
        m1_phase = 0;
        s1_phase = 0;
        m1_phase_increment = 0;
        s1_phase_increment = 0;
        
        pitch_interpolator = ParameterInterpolator();
        formant_interpolator = ParameterInterpolator();
        
        pitch_ = 0;
        formant_ = 0;
    }
    
    //    ~Voice() {
    //        if(audioBuffer) delete (audioBuffer);
    //    }
    
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
        return mVCAEnv.getEnvelopeState() == ADSREnvelope::kOff;
        //        return !mNoteOn;
    }
    
    double process(float input) {
        double sample = 0.0f;
        
        //        if(isAnyNoteOn()) {
        
        
//        test_noxfade_overwrite_buffer(&sample, input);
//        test_xfade_japan(&sample, input);
        test_xfade_japan_master_phase_increment(&sample, input);
        //
        //        } else {
        //            sample = input;
        //        }
        //        noteOnTrigger = false;
        //        noteOffTrigger = false;
        
        //        float vcfEnvelopeControlVoltage = mVCFEnv.process();
        
        //        float filterStage1Output = mResonantFilter.process(oscillatorOutput, vcfEnvelopeControlVoltage, mNote);
        //        float filterState2Output = mResonantFilter2.process(filterStage1Output, vcfEnvelopeControlVoltage, mNote);
        //        float filterState2Output = svf.Process<FILTER_MODE_LOW_PASS>(oscillatorOutput, vcfEnvelopeControlVoltage, mNote);
        
        //        return 0;
        float vcaEnvelopeOutput = mVCAEnv.process() * sample;
        
        return vcaEnvelopeOutput;
    }
    
    void noteOn(int note) {
        if(note != mNote) {
            mNote = note;
            
            float shifted_note = note - 24.0f;
            resetAudioBuffer(1, ceil(mSampleRate / midiNoteToFrequency(shifted_note)));
        }
        
        mVCAEnv.noteOn();
        m1_phase = 0;
        s1_phase = 0;
        //        mVCFEnv.noteOn();
    }
    
    void noteOff(int note) {
        mVCAEnv.noteOff();
        //        mVCFEnv.noteOff();
        //        mNoteOn = false;
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
    
    bool isAudioBufferFull() {
        return writeSampleIndex >= audioBuffer.getNumSamples();
    }
    
    bool isXFadeBufferFull() {
        return xfadeSampleIndex >= audioBuffer.getNumSamples() * mSynthParams->xfade;
    }
    
    float writeXFadeBuffer(float data) {
        if(xfadeSampleIndex < audioBuffer.getNumSamples() * mSynthParams->xfade) {
            float currentSample = audioBuffer.getSample(0, xfadeSampleIndex);
            float mixTop = float(xfadeSampleIndex) / (audioBuffer.getNumSamples() * mSynthParams->xfade);
            float newSample = mixTop * data + (1-mixTop) * currentSample;
            audioBuffer.setSample(0, xfadeSampleIndex++, newSample);
            return newSample;
        }
        return 0.0;
    }
    
    
    void test_noxfade_overwrite_buffer(double* sample, float input) {
        
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
        
        
        float pitch = mNote + (pitch_ * 48.0f - 24.0f);
        //        float formant = mNote + (formant_ * 48.0f - 24.0f);
        float formant = mNote + 48 * (formant_ + pitch_ - 1);

        m1_phase_increment = midiNoteToFrequency(pitch) / mSampleRate;
        s1_phase_increment = midiNoteToFrequency(formant) / mSampleRate;
        
        if (writeSampleIndex > 1 / m1_phase_increment && writeSampleIndex > 1 / s1_phase_increment)
        {
            m1_phase += m1_phase_increment;
            s1_phase += s1_phase_increment;
            
            if(m1_phase >= 1.0) {
                m1_phase -= 1.0;
                
                double t = m1_phase / m1_phase_increment;
                s1_phase = t * s1_phase_increment;
            }
            
            if(s1_phase >= 1.0) {
                s1_phase -= 1.0;
            }
            
            float t1 = s1_phase / s1_phase_increment;
            
            *sample = getInterpolatedSample(t1);
            
        } else {
            *sample = input;
        }
    }
    
    void test_xfade_overwrite_buffer(double* sample, float input) {
        if(isAudioBufferFull()) {
            
            if(isXFadeBufferFull()) {
                
                float calculatedNote;
                
                calculatedNote = mNote+(mSynthParams->pitch * 48.0f - 24.0f);
                m1_phase_increment = midiNoteToFrequency(calculatedNote) / mSampleRate;
                
                calculatedNote = mNote+(mSynthParams->formant * 48.0f - 24.0f);
                //                calculatedNote = clamp(calculatedNote, 0.0, 127.0);
                s1_phase_increment = midiNoteToFrequency(calculatedNote) / mSampleRate;
                
                m1_phase += m1_phase_increment;
                s1_phase += s1_phase_increment;
                
                if(m1_phase >= 1.0) {
                    m1_phase -= 1.0;
                    //                    m1_phase = fmod(m1_phase, 1.0);
                    s1_phase = 0;
                }
                
                if(s1_phase >= 1.0) {
                    s1_phase -= 1.0;
                    //                    s1_phase = fmod(s1_phase, 1.0);
                }
                
                *sample = getInterpolatedSample(s1_phase);
                
            } else {
                *sample = writeXFadeBuffer(input);
            }
        } else {
            writeAudioBuffer(input);
            *sample = input;
        }
    }
    
    void test_xfade_japan(double* sample, float input) {
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
        
        
        float pitch = mNote + (pitch_ * 48.0f - 24.0f);
        //        float formant = mNote + (formant_ * 48.0f - 24.0f);
        float formant = mNote + 48 * (formant_ + pitch_ - 1);
        
        m1_phase_increment = midiNoteToFrequency(pitch) / mSampleRate;
        s1_phase_increment = midiNoteToFrequency(formant) / mSampleRate;
        
        if (writeSampleIndex > 1 / m1_phase_increment && writeSampleIndex > 1 / s1_phase_increment)
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
                    
                    float t1 = s1_phase / s1_phase_increment;
                    float tn = sn_phase / s1_phase_increment;
                    
                    float s1_output = getInterpolatedSample(t1);
                    float sn_output = getInterpolatedSample(tn);
                    
                    float fade_sn = sn_phase / mSynthParams->xfade;
                    output1 = fade_sn * sn_output + (1-fade_sn) * s1_output;
                    // do formant mix
                } else {
                    // no formant mix
                    float t1 = s1_phase / s1_phase_increment;
                    
                    output1 = getInterpolatedSample(t1);
                }
                
                if(s2_phase >= 1.0 + mSynthParams->xfade) {
                    s2_phase -= 1.0;
                }
                if(s2_phase >= 1.0 && mSynthParams->xfade > 0.0) {
                    // we are in a region of two formants overlapping
                    // cross fade m2's s2 with itself
                    float sn_phase = s2_phase - 1.0;
                    
                    float t2 = s2_phase / s1_phase_increment;
                    float tn = sn_phase / s1_phase_increment;
                    
                    float s2_output = getInterpolatedSample(t2);
                    float sn_output = getInterpolatedSample(tn);
                    
                    float fade_sn = sn_phase / mSynthParams->xfade;
                    output2 = fade_sn * sn_output + (1-fade_sn) * s2_output;
                    // do formant mix
                } else {
                    // no formant mix
                    float t2 = s2_phase / s1_phase_increment;
                    
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
                    
                    float t1 = s1_phase / s1_phase_increment;
                    float tn = sn_phase / s1_phase_increment;
                    
                    float s1_output = getInterpolatedSample(t1);
                    float sn_output = getInterpolatedSample(tn);
                    
                    float fade_sn = sn_phase / mSynthParams->xfade;
                    *sample = fade_sn * sn_output + (1-fade_sn) * s1_output;
                    // do formant mix
                } else {
                    // no formant mix
                    float t1 = s1_phase / s1_phase_increment;
                    
                    *sample = getInterpolatedSample(t1);
                }
            }
        } else {
            *sample = input;
        }
    }
    
    void test_xfade_japan_master_phase_increment(double* sample, float input) {
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
        
        
        float pitch = mNote + (pitch_ * 48.0f - 24.0f);
        //        float formant = mNote + (formant_ * 48.0f - 24.0f);
        float formant = mNote + 48 * (formant_ + pitch_ - 1);
        
        m1_phase_increment = midiNoteToFrequency(pitch) / mSampleRate;
        s1_phase_increment = midiNoteToFrequency(formant) / mSampleRate;
        
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
    ParameterInterpolator pitch_interpolator;
    ParameterInterpolator formant_interpolator;
    
    //    Oscillator mOsc1;
    //    Oscillator mOsc2;
    ADSREnvelope mVCAEnv;
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
    
    double pitch_;
    double formant_;
};

#endif /* Voice_h */

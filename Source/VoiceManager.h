//
//  VoiceManager.h
//  NewProject - Shared Code
//
//  Created by Eugene Clewlow on 7/16/23.
//

#ifndef VoiceManager_h
#define VoiceManager_h

#include "Voice.h"
#include "SynthParams.h"

#define MAX_VOICES 6

static const bool voice_is_off (const Voice v)
{
    return v.isFinished();
}

static inline double clamp(double input, double low, double high) {
    return std::min(std::max(input, low), high);
}

class InstrumentExtensionDSPKernel;

class VoiceManager {
public:
    
    VoiceManager(double sampleRate = 44100.0, SynthParams* synthParams={}){
        mSampleRate = sampleRate;
        mSynthParams = synthParams;
        audioBuffer = new juce::AudioBuffer<float>();
        resetAudioBuffer(1, 0);
    }
    
    void setSampleRate(double newSampleRate) {
        mSampleRate = newSampleRate;
    }
    
    void recomputeFrequency() {
        std::list<Voice>::iterator it;
        
        for (it = mVoiceList.begin(); it!= mVoiceList.end(); it++) {
            (*it).recomputeFrequency();
        }
        
    }
    
    void reset() {
        std::list<Voice>::iterator it;
        
        for (it = mVoiceList.begin(); it!= mVoiceList.end(); it++) {
            (*it).reset();
        }
    }
    
    double process(float input) {
        double sample = 0.0f;
        
        if(isAnyNoteOn()) {
                        
            if(isAudioBufferFull()) {
                float calculatedNote;
                
                calculatedNote = note_+(mSynthParams->pitch * 48.0f - 24.0f);
                calculatedNote = clamp(calculatedNote, 0.0, 127.0);
                phaseIncrement = midiNoteToFrequency(calculatedNote) / mSampleRate;
                
                phase += phaseIncrement;
                if(phase >= 1.0)
                    phase -= 1.0;

                float t = phase / masterPhaseIncrement;

                sample = getInterpolatedSample(t);

                calculatedNote = note_;//+(mSynthParams->pitch * 48.0f - 24.0f);
                calculatedNote = clamp(calculatedNote, 0.0, 127.0);
                m1_phase_increment = midiNoteToFrequency(calculatedNote) / mSampleRate;

                calculatedNote = note_+(mSynthParams->formant * 48.0f - 24.0f);
                calculatedNote = clamp(calculatedNote, 0.0, 127.0);
                s1_phase_increment = midiNoteToFrequency(calculatedNote) / mSampleRate;

                m1_phase += m1_phase_increment;
                s1_phase += s1_phase_increment;
                
                if(m1_phase >= 1.0) {
                    m1_phase -= 1.0;
                    s1_phase = 0;
                }
                
                if(s1_phase >= 1.0) {
                    s1_phase -= 1.0;
                }

                t = s1_phase / masterPhaseIncrement;

                sample = getInterpolatedSample(t);

                
//                if(isXFadeAudioBufferFull()) {
//                        // do xfade stuff here.
//                    float calculatedNote;
//
//                    calculatedNote = note_+(mSynthParams->pitch * 48.0f - 24.0f);
//                    calculatedNote = clamp(calculatedNote, 0.0, 127.0);
//                    m1_phase_increment = midiNoteToFrequency(calculatedNote) / mSampleRate;
//
//                    calculatedNote = note_+(mSynthParams->formant * 48.0f - 24.0f);
//                    calculatedNote = clamp(calculatedNote, 0.0, 127.0);
//                    s1_phase_increment = midiNoteToFrequency(calculatedNote) / mSampleRate;
//
//                    m1_phase += m1_phase_increment;
//                    s1_phase += s1_phase_increment;
//
//                    if(m1_phase >= 1.0 + mSynthParams->xfade) {
//                        m1_phase -= 1.0;
//                        float t = m1_phase / m1_phase_increment;
//                        s1_phase = t * s1_phase_increment;
//                        s1_phase = fmod(s1_phase, 2.0);
//                    }
//
//                    if(m1_phase >= 1.0 && mSynthParams->xfade > 0.0) {
//                        float m2_phase = m1_phase - 1.0;
//                        float t = m2_phase / m1_phase_increment;
//                        float s2_phase = t * s1_phase_increment;
//                        s2_phase = fmod(s2_phase, 2.0);
//
//                        float output1;
//                        float output2;
//                        // we are in a fade region of two masters overlapping.
//
//                        if(s1_phase >= 1.0 + mSynthParams->xfade) {
//                            s1_phase -= 1.0;
//                        }
//                        if(s1_phase >= 1.0 && mSynthParams->xfade > 0.0) {
//                            // we are in a region of two formants overlapping on itself
//                            // cross fade m1's s1 with itself
//                            float sn_phase = s1_phase - 1.0;
//
//                            float t1 = s1_phase / masterPhaseIncrement;
//                            float tn = sn_phase / masterPhaseIncrement;
//
//                            float s1_output = getInterpolatedSample(t1);
//                            float sn_output = getInterpolatedSample(tn);
//
//                            float fade_sn = sn_phase / mSynthParams->xfade;
//                            output1 = fade_sn * sn_output + (1-fade_sn) * s1_output;
//                            // do formant mix
//                        } else {
//                            // no formant mix
//                            output1 = getInterpolatedSample(s1_phase / masterPhaseIncrement);
//                        }
//
//                        if(s2_phase >= 1.0 + mSynthParams->xfade) {
//                            s2_phase -= 1.0;
//                        }
//                        if(s2_phase >= 1.0 && mSynthParams->xfade > 0.0) {
//                            // we are in a region of two formants overlapping
//                            // cross fade m2's s2 with itself
//                            float sn_phase = s2_phase - 1.0;
//
//                            float t2 = s2_phase / masterPhaseIncrement;
//                            float tn = sn_phase / masterPhaseIncrement;
//
//                            float s2_output = getInterpolatedSample(t2);
//                            float sn_output = getInterpolatedSample(tn);
//
//                            float fade_sn = sn_phase / mSynthParams->xfade;
//                            output2 = fade_sn * sn_output + (1-fade_sn) * s2_output;
//                            // do formant mix
//                        } else {
//                            // no formant mix
//                            output2 = getInterpolatedSample(s2_phase / masterPhaseIncrement);
//                        }
//
//                        float fade_m2 = m2_phase / mSynthParams->xfade;
//                        sample = fade_m2 * output2 + (1-fade_m2) * output1;
//
//                    } else {
//                        // no master cross fade
//
//                        if(s1_phase >= 1.0 + mSynthParams->xfade) {
//                            s1_phase -= 1.0;
//                        }
//                        if(s1_phase >= 1.0 && mSynthParams->xfade > 0.0) {
//                            // we are in a region of two formants overlapping on itself
//                            // cross fade m1's s1 with itself
//                            float sn_phase = s1_phase - 1.0;
//
//                            float t1 = s1_phase / masterPhaseIncrement;
//                            float tn = sn_phase / masterPhaseIncrement;
//
//                            float s1_output = getInterpolatedSample(t1);
//                            float sn_output = getInterpolatedSample(tn);
//
//                            float fade_sn = sn_phase / mSynthParams->xfade;
//                            sample = fade_sn * sn_output + (1-fade_sn) * s1_output;
//                            // do formant mix
//                        } else {
//                            // no formant mix
//                            sample = getInterpolatedSample(s1_phase / masterPhaseIncrement);
//                        }
//                    }
//                } else {
//                    writeAudioBuffer(input);
//                }
                // start
            } else {
                writeAudioBuffer(input);
                sample = input;
            }
        } else {
            sample = input;
        }
        
        return sample;
    }
    
    inline double getInterpolatedSample(double t) {
        int integral = static_cast<int>(t);
        float fractional = t - static_cast<float>(integral);
        
        // depending on position of integral within audioBuffer
        // 118.5 out of 119, reduce
        // also 0.1 out of 119 reduce
        // the further in you go, the less reduced it is
        // so
        //  if integral >= numsamples / 2
        //      then (numsamples - integral), = 119 - 60 = 59
        //              59...0 as integral increases
        //          divide by numsamples / 2
        //              1.0 ... 0
        // if integral < numsamples / 2
        //      then (numsamples /2 - integral) = 50 0...50
        //      0...50 as integral increases
        // mSynthParams->xfade
//        float n = audioBuffer->getNumSamples();
//        float fade_region = mSynthParams->xfade * n / 2.0;
        
        float mix = 1.0;
        
//        if(t < fade_region) {
//            mix = 1.0f - (fade_region - t) / fade_region;
//        }
//
//        if(t > (n - fade_region)) {
//            mix = 1.0f - (t - (n - fade_region)) / fade_region;
//        }
        
        float s0 = audioBuffer->getSample(0, integral);
        float s1 = audioBuffer->getSample(0, integral + 1);
        
        float f = s0 + (s1 - s0) * fractional;
        return mix * f;
    }
    
    inline double midiNoteToFrequency(double note) {
        constexpr auto kMiddleA = 440.0;
        //        double pitchBend = (mSynthParams->pitch_bend - 0x40) * 12.0 / 0x40;
        double pitchBend = 0;
        note = clamp(note, 0.0, 127.0);
        return (kMiddleA / 32.0) * pow(2, (((note+pitchBend) - 9.0) / 12.0));
    }
    
    
    void noteOn(int note) {
        std::list<Voice>::iterator it;
        
        mVoiceList.remove_if(voice_is_off);
                
        int foundNoteIndex = -1;
        
        int i = 0;
        for (it = mVoiceList.begin(); it!= mVoiceList.end(); it++) {
            if((*it).getNote() == note) {
                //                (*it).noteOff(note);
                (*it).noteOn(note);
                foundNoteIndex = i;
                break;
            }
            i++;
        }
        
        if(foundNoteIndex < 0) {
            if(isAllNotesOff()) {
                float shifted_note = note + (mSynthParams->pitch * 48.0f - 24.0f);
                resetAudioBuffer(1, ceil(mSampleRate / midiNoteToFrequency(shifted_note)));
                masterPhaseIncrement = midiNoteToFrequency(shifted_note) / mSampleRate;
                
                note_ = shifted_note;
            }
            
            Voice v = Voice(mSampleRate, mSynthParams);
            v.noteOn(note);
            mVoiceList.push_back(v);
        }
    }
    
    void setFrequency(double semitones) {
        float calculatedNote = note_+semitones;
        calculatedNote = clamp(calculatedNote, 0.0, 127.0);
        phaseIncrement = midiNoteToFrequency(calculatedNote) / mSampleRate;
    }
    
    void resetAudioBuffer(int channel, int size) {
        audioBuffer->clear();
        audioBuffer->setSize(channel, size);
        writeSampleIndex = 0;
        phase = 0;
    }
    
    void writeAudioBuffer(float data) {
        if(writeSampleIndex < audioBuffer->getNumSamples()) {
            audioBuffer->setSample(0, writeSampleIndex++, data);
        }
    }
        
    bool isAudioBufferFull() {
        return writeSampleIndex >= audioBuffer->getNumSamples();
    }
    
    void allNotesOff() {
        std::list<Voice>::iterator it;
        for (it = mVoiceList.begin(); it!= mVoiceList.end(); it++) {
            (*it).noteOff(0);
        }
    }
    
    bool isAnyNoteOn() {
        mVoiceList.remove_if(voice_is_off);
        
        return !mVoiceList.empty();
    }
    
    bool isAllNotesOff() {
        mVoiceList.remove_if(voice_is_off);
        
        return mVoiceList.empty();
    }
    
    void noteOff(int note) {
        std::list<Voice>::iterator it;
        for (it = mVoiceList.begin(); it!= mVoiceList.end(); it++) {
            
            if((*it).getNote() == note) {
                (*it).noteOff(note);
            }
        }
        
        if(isAllNotesOff()) {
            resetAudioBuffer(1, 0);
            note_ = -1;
        }
    }
    
private:
    double mSampleRate = { 0.0 };
    double phase;
    double phaseIncrement;
    double masterPhaseIncrement;
    double note_;
    double m1_phase;
    double s1_phase;
    double m1_phase_increment;
    double s1_phase_increment;
    SynthParams* mSynthParams;
    std::list<Voice>mVoiceList;
    juce::AudioBuffer<float>* audioBuffer;
    uint writeSampleIndex;
};


#endif /* VoiceManager_h */

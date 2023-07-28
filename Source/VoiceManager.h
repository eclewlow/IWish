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
    }
    
    void setSampleRate(double newSampleRate) {
        mSampleRate = newSampleRate;
    }

    
    void reset() {
        std::list<Voice>::iterator it;
        
        for (it = mVoiceList.begin(); it!= mVoiceList.end(); it++) {
            (*it).reset();
        }
    }
    
    
    double process(float input) {
        std::list<Voice>::iterator it;
        
        double sample = 0.0f;
        for (it = mVoiceList.begin(); it!= mVoiceList.end(); it++) {
            sample += (*it).process(input);
        }

        return sample;
    }

    double getEnvelopeGain() {
        std::list<Voice>::iterator it;
        
        double max = 0.0;

        for (it = mVoiceList.begin(); it!= mVoiceList.end(); it++) {
            max = std::max((*it).getEnvelopeGain(), max);
        }

        return max;
    }
    
    void noteOff(int note) {
        std::list<Voice>::iterator it;
        for (it = mVoiceList.begin(); it!= mVoiceList.end(); it++) {
            
            if((*it).getNote() == note) {
                (*it).noteOff(note);
            }
        }
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
            Voice v = Voice(mSampleRate, mSynthParams);
            v.noteOn(note);
            mVoiceList.push_back(v);
        }
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
    
    

    
private:
    double mSampleRate = { 0.0 };
    SynthParams* mSynthParams;
    std::list<Voice>mVoiceList;
};


#endif /* VoiceManager_h */

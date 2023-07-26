//
//  SynthParams.h
//  NewProject - Shared Code
//
//  Created by Eugene Clewlow on 7/16/23.
//

#ifndef SynthParams_h
#define SynthParams_h


enum OscillatorMode {
    OSCILLATOR_MODE_SINE = 0,
    OSCILLATOR_MODE_SAW = 1,
    OSCILLATOR_MODE_SQUARE = 2,
    OSCILLATOR_MODE_TRIANGLE = 3,
    OSCILLATOR_MODE_FM = 4
};

typedef struct SynthParams {
    float vca_attack = 10.0;
    float vca_decay = 0.0;
    float vca_sustain = 1.0;
    float vca_release = 1000.0;

    float vcf_attack = 10.0;
    float vcf_decay = 0.0;
    float vcf_sustain = 1.0;
    float vcf_release = 100.0;
    
    float pitch_envelope_delay = 8.0;
    float pitch_envelope_attack = 8.0;
    float pitch_envelope_curve = 1.0;
    
    float pitch_envelope_amout = 0.0;
    float formant_envelope_amount = 0.0;
    
    float vcf_envelope_amount = 0.0;
    float vcf_keyboard_tracking_amount = 0.0;

    float cutoff = 8500.0;
    float resonance = -8.0;

    float fine_tune = 0.0;
    float coarse_tune = 0.0;

    uint pitch_bend = 0x40;
    
    float xfade = 0.0;
    float formant = 0.0;
    float pitch = 0.0;
    
    OscillatorMode oscillator_mode = OSCILLATOR_MODE_SAW;
    
    float fm_ratio = 1.0;
    
    float fm_gain = 0.0;
    float fm_feedback = 0.0;
    
    float pulse_width = 50;
    
    bool hard_sync = false;
    float bitcrush_rate = 48000.0;

} SynthParams;


#endif /* SynthParams_h */

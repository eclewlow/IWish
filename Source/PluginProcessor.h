/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "VoiceManager.h"
#include "SynthParams.h"

//==============================================================================
/**
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    juce::AudioParameterFloat* pitch;
    juce::AudioParameterFloat* formant;
    juce::AudioParameterFloat* pitchOffset;
    juce::AudioParameterFloat* formantOffset;
    juce::AudioParameterBool* link;
    juce::AudioParameterFloat* xfade;
    juce::AudioParameterFloat* mix;
    juce::AudioParameterFloat* attack;
    juce::AudioParameterFloat* release;
    juce::AudioParameterFloat* pitch_envelope_delay;
    juce::AudioParameterFloat* pitch_envelope_attack;
    juce::AudioParameterFloat* pitch_envelope_curve;
    juce::AudioParameterFloat* pitch_envelope_amount;
    juce::AudioParameterFloat* formant_envelope_amount;
    juce::AudioParameterFloat* pitch_envelope_link_offset;
    juce::AudioParameterFloat* formant_envelope_link_offset;

    bool mute;
    juce::MidiKeyboardState* keyboardState;
    VoiceManager mVoiceManager;
    SynthParams synthParams;

    double xfade_phase;
    double xfade_phase_increment;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
};

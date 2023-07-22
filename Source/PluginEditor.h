/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class NewProjectAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::AudioProcessorParameter* getParameter (const juce::String& paramId);
    float getParameterValue (const juce::String& paramId);
    void setParameterValue (const juce::String& paramId, float value);
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;

    juce::Slider pitchKnob;
    juce::Slider formantKnob;
    juce::Label pitchLabel;
    juce::Label formantLabel;
    juce::Slider xfadeSlider;
    juce::Label xfadeLabel;
    juce::Slider mixSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;

    juce::TextButton linkButton;
    
    juce::Slider xFadeSlider;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};

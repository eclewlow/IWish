/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                  .withInput  ("Sidechain", juce::AudioChannelSet::stereo())
                  )
#endif
{
    addParameter (pitch = new juce::AudioParameterFloat ({ "pitch", 1 }, "Pitch", juce::NormalisableRange<float> (0.0f, 1.0f), 0.5));
    addParameter (formant = new juce::AudioParameterFloat ({ "formant", 1 }, "Formant", juce::NormalisableRange<float> (0.0f, 1.0f), 0.5));
    addParameter (pitchOffset = new juce::AudioParameterFloat ({ "pitchOffset", 1 }, "Pitch Offset", juce::NormalisableRange<float> (0.0f, 1.0f), 0.5));
    addParameter (formantOffset = new juce::AudioParameterFloat ({ "formantOffset", 1 }, "Formant Offset", juce::NormalisableRange<float> (0.0f, 1.0f), 0.5));
    addParameter (link = new juce::AudioParameterBool ({ "link", 1 }, "Link", false));
    
    addParameter (xfade = new juce::AudioParameterFloat ({ "xfade", 1 }, "X-Fade", 0.0f, 1.0f, 0.0));
    addParameter (mix = new juce::AudioParameterFloat ({ "mix", 1 }, "Mix", juce::NormalisableRange<float> (0.0f, 1.0f), 0.5));
    mute = true;
    keyboardState = new juce::MidiKeyboardState();
    mVoiceManager = VoiceManager(getSampleRate(), &synthParams);
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    //    auto mainInputOutput = getBusBuffer (buffer, true, 0);
    auto sideChainInput  = getBusBuffer (buffer, true, 0);
    
    mVoiceManager.setSampleRate(getSampleRate());
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int j = 0; j < buffer.getNumSamples(); ++j)
    {
        juce::MidiBufferIterator midiIterator =  midiMessages.findNextSamplePosition(j);
        while(midiIterator != midiMessages.cend()) {
            const auto metadata = *midiIterator;
            if(metadata.samplePosition > j) break;
            
            juce::MidiMessage midiMessage = metadata.getMessage();
            if(midiMessage.isNoteOn()) {
                // start recording here
                //                mute = false;
                mVoiceManager.noteOn(midiMessage.getNoteNumber());
            }
            if(midiMessage.isNoteOff())// || midiMessage.isNoteOff())
            {
                // stop gate and play original.
                mVoiceManager.noteOff(midiMessage.getNoteNumber());
            }
            ++midiIterator;
        }
        
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            //            if(channel < sideChainInput.getNumChannels() && channel < totalNumOutputChannels) {
            
            if(channel < sideChainInput.getNumChannels()) {
                
                // mix 0 full wet 0.0...1.0
                // mix 100 fully dry
                float wet = mix->get();

                float dry;
                
                if(mVoiceManager.isAllNotesOff()) {
                    if(wet >= 0.5) {
                        // i want 1 to 0
                        // wet from 0.5 ... 1 to go from 1 to 0
                        // so subtract 0.5,  so 0 to .5
                        // then multiply by 2, so 0 to 1
                        // then 1-ans = 1 to 0
                        // so 1-2*(wet-0.5)
                        buffer.getWritePointer (channel)[j] = (1-2*(wet-0.5))*sideChainInput.getReadPointer (channel)[j];
                    } else {
                        buffer.getWritePointer (channel)[j] = sideChainInput.getReadPointer (channel)[j];
                    }
                }
                else {
                    //                    mVoiceManager.setFrequency(pitch->get() * 48.0f - 24.0f);
                    synthParams.pitch = pitch->get();
                    synthParams.formant = formant->get();
                    synthParams.xfade = xfade->get();
                    
                    if(wet <= 0.5) {
                        // wet is 0...0.5
                        // went from 0 ... 0.5 to go from 0 to 1
                        // so wet *2
                        buffer.getWritePointer (channel)[j] = (wet*2)*mVoiceManager.process( sideChainInput.getReadPointer (channel)[j]);
                    } else {
                        buffer.getWritePointer (channel)[j] = mVoiceManager.process( sideChainInput.getReadPointer (channel)[j]);
                    }
                }
            }
            
            else
                buffer.getWritePointer (channel)[j] = 0;
            //            }
            
            // ..do something to the data...
        }
    }
    
    //
    //    auto alphaCopy     = alpha->get();
    //    auto thresholdCopy = threshold->get();
    //
    //    for (int j = 0; j < buffer.getNumSamples(); ++j)
    //    {
    //        auto mixedSamples = 0.0f;
    //
    //        for (int i = 0; i < sideChainInput.getNumChannels(); ++i)
    //            mixedSamples += sideChainInput.getReadPointer (i)[j];
    //
    //
    //        // very in-effective way of doing this
    //        for (int i = 0; i < mainInputOutput.getNumChannels(); ++i)
    //            *mainInputOutput.getWritePointer (i, j) = sampleCountDown > 0 ? *mainInputOutput.getReadPointer (i, j) : 0.0f;
    //
    //        if (sampleCountDown > 0)
    //            --sampleCountDown;
    //    }
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor (*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream stream (destData, true);
    
    stream.writeFloat (*pitch);
    stream.writeFloat (*formant);
    stream.writeFloat (*pitchOffset);
    stream.writeFloat (*formantOffset);
    stream.writeBool (*link);
    stream.writeFloat (*xfade);
    stream.writeFloat (*mix);
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);
    
    pitch->setValueNotifyingHost   (stream.readFloat());
    formant->setValueNotifyingHost (stream.readFloat());
    pitchOffset->setValueNotifyingHost (stream.readFloat());
    formantOffset->setValueNotifyingHost (stream.readFloat());
    link->setValueNotifyingHost    (stream.readBool());
    xfade->setValueNotifyingHost (stream.readFloat());
    mix->setValueNotifyingHost (stream.readFloat());
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}

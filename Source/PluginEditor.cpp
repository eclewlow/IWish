/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    pitchKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    if((float)getParameterValue ("pitch") * 48.0f - 24.0f == floor((float)getParameterValue ("pitch") * 48.0f - 24.0f))
        pitchKnob.setRange (-24, 24, 1);
    else
        pitchKnob.setRange (-24, 24, 0.01);
    pitchKnob.setPopupMenuEnabled (true);
    pitchKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 70, 20);
    pitchKnob.setTextValueSuffix (" semi");
    pitchKnob.setValue ((float) getParameterValue ("pitch") * 48.0f - 24.0f, juce::NotificationType::dontSendNotification);
    pitchKnob.onValueChange = [this] {
        setParameterValue ("pitch", (float) (pitchKnob.getValue() + 24.0)/48.0f);
        if((bool)getParameterValue("link")) {
            float diff = (float)getParameterValue("pitch") - (float)getParameterValue("pitchOffset");
            float newFormant = diff + (float)getParameterValue("formantOffset");
            formantKnob.setValue (newFormant * 48.0f - 24.0f, juce::NotificationType::dontSendNotification);
            setParameterValue ("formant", (float) (formantKnob.getValue() + 24.0)/48.0f);
        }
    };
    pitchKnob.setDoubleClickReturnValue(true, 0.0);
    pitchKnob.onDragStart = [this] {
        if(juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown()) {
            if((bool)getParameterValue("link")) {
                formantKnob.setRange (-24, 24, 0.01);
            }
            pitchKnob.setRange (-24, 24, 0.01);
        } else {
            if((bool)getParameterValue("link")) {
                formantKnob.setRange (-24, 24, 1);
            }
            pitchKnob.setRange (-24, 24, 1);
        }
    };

    
    pitchLabel.setText("Pitch", juce::NotificationType::dontSendNotification);
    pitchLabel.setInterceptsMouseClicks(false, false);
    pitchLabel.setJustificationType(juce::Justification::centred);
    
    formantKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    if((float)getParameterValue ("formant") * 48.0f - 24.0f == floor((float)getParameterValue ("formant") * 48.0f - 24.0f))
        formantKnob.setRange (-24, 24, 1);
    else
        formantKnob.setRange (-24, 24, 0.01);
    formantKnob.setPopupMenuEnabled (true);
    formantKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 70, 20);
    formantKnob.setTextValueSuffix (" semi");
    formantKnob.setValue ((float)getParameterValue ("formant") * 48.0f - 24.0f, juce::NotificationType::dontSendNotification);
    formantKnob.onValueChange = [this] {
        setParameterValue ("formant", (float) (formantKnob.getValue() + 24.0)/48.0f);
        if((bool)getParameterValue("link")) {
            setParameterValue ("formantOffset", getParameterValue("formant"));
            setParameterValue ("pitchOffset", getParameterValue("pitch"));
        }
    };
    formantKnob.setDoubleClickReturnValue(true, 0.0);
    formantKnob.onDragStart = [this] {
        if(juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown()) {
            formantKnob.setRange (-24, 24, 0.01);
        } else {
            formantKnob.setRange (-24, 24, 1);
        }
    };

    formantLabel.setText("Formant", juce::NotificationType::dontSendNotification);
    formantLabel.setInterceptsMouseClicks(false, false);
    formantLabel.setJustificationType(juce::Justification::centred);
    
    linkButton.setButtonText("Link");
    
    if((bool)getParameterValue("link"))
        linkButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
    else
        linkButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    linkButton.onClick = [this] { setParameterValue("link", !(bool)getParameterValue("link"));
        if((bool)getParameterValue("link")) {
            linkButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
            setParameterValue ("formantOffset", getParameterValue("formant"));
            setParameterValue ("pitchOffset", getParameterValue("pitch"));
            setParameterValue ("formant_envelope_link_offset", getParameterValue("formant_envelope_amount"));
            setParameterValue ("pitch_envelope_link_offset", getParameterValue("pitch_envelope_amount"));
        }
        else
            linkButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    };
    
    xfadeSlider.setRange (0, 100, 0.01);
    xfadeSlider.setSliderStyle (juce::Slider::LinearBar);
    //    pitchKnob.setPopupMenuEnabled (true);
    xfadeSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 70, 60);
    xfadeSlider.setTextValueSuffix ("%");
    xfadeSlider.setValue ((float) getParameterValue ("xfade") * 100.0, juce::NotificationType::dontSendNotification);
    xfadeSlider.onValueChange = [this] {
        setParameterValue ("xfade", (float) xfadeSlider.getValue() /100.0f);
    };
    
    xfadeLabel.setText("X-Fade", juce::NotificationType::dontSendNotification);
    xfadeLabel.setInterceptsMouseClicks(false, false);
    xfadeLabel.setJustificationType(juce::Justification::centred);

    
    mixSlider.setRange (0, 100, 0.1);
    mixSlider.setSliderStyle (juce::Slider::LinearVertical);
    //    pitchKnob.setPopupMenuEnabled (true);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 200, 30);
    mixSlider.setTextValueSuffix ("%");
    mixSlider.setValue ((float) getParameterValue ("mix") * 100.0, juce::NotificationType::dontSendNotification);
    mixSlider.onValueChange = [this] {
        setParameterValue ("mix", (float) mixSlider.getValue() /100.0f);
    };
    
    attackSlider.setRange (0, 1.0, 0.001);
    attackSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    attackSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    attackSlider.setValue ((float) getParameterValue ("attack"), juce::NotificationType::dontSendNotification);
    attackSlider.onValueChange = [this] {
        setParameterValue ("attack", (float) attackSlider.getValue());
    };

    releaseSlider.setRange (0, 1.0, 0.001);
    releaseSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    releaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    releaseSlider.setValue ((float) getParameterValue ("release"), juce::NotificationType::dontSendNotification);
    releaseSlider.onValueChange = [this] {
        setParameterValue ("release", (float) releaseSlider.getValue());
    };

    
    envelopeComponent.setAttack ((float) pow(10, getParameterValue ("pitch_envelope_attack")) , juce::NotificationType::dontSendNotification);
    envelopeComponent.setAttackChangeListener([this] {
        setParameterValue ("pitch_envelope_attack", (float) log10(envelopeComponent.getAttack()));
    });

    envelopeComponent.setDelay ((float) getParameterValue ("pitch_envelope_delay"), juce::NotificationType::dontSendNotification);
    envelopeComponent.setDelayChangeListener([this] {
        setParameterValue ("pitch_envelope_delay", (float)envelopeComponent.getDelay());
    });

    envelopeComponent.setCurve ((float) getParameterValue ("pitch_envelope_curve"), juce::NotificationType::dontSendNotification);
    envelopeComponent.setCurveChangeListener([this] {
        setParameterValue ("pitch_envelope_curve", (float) envelopeComponent.getCurve());
    });

    envelopeComponent.setPitchEnvelopeAmount ((float) getParameterValue ("pitch_envelope_amount"), juce::NotificationType::dontSendNotification);
    envelopeComponent.setPitchEnvelopeAmountChangeListener([this] {
        setParameterValue ("pitch_envelope_amount", (float) envelopeComponent.getPitchEnvelopeAmount());

        if((bool)getParameterValue("link")) {
            float diff = (float)getParameterValue("pitch_envelope_amount") - (float)getParameterValue("pitch_envelope_link_offset");
            float newFormant = diff + (float)getParameterValue("formant_envelope_link_offset");
            envelopeComponent.setFormantEnvelopeAmount (newFormant, juce::NotificationType::dontSendNotification);
            setParameterValue ("formant_envelope_amount", (float) envelopeComponent.getFormantEnvelopeAmount());
        }
    });

    envelopeComponent.setFormantEnvelopeAmount ((float) getParameterValue ("formant_envelope_amount"), juce::NotificationType::dontSendNotification);
    envelopeComponent.setFormantEnvelopeAmountChangeListener([this] {
        setParameterValue ("formant_envelope_amount", (float) envelopeComponent.getFormantEnvelopeAmount());
        if((bool)getParameterValue("link")) {
            setParameterValue ("formant_envelope_link_offset", getParameterValue("formant_envelope_amount"));
            setParameterValue ("pitch_envelope_link_offset", getParameterValue("pitch_envelope_amount"));
        }
    });

      
    
    addAndMakeVisible (pitchKnob);
    addAndMakeVisible(pitchLabel);
    addAndMakeVisible (formantKnob);
    addAndMakeVisible(formantLabel);
    addAndMakeVisible (linkButton);
    addAndMakeVisible(xfadeSlider);
    addAndMakeVisible(xfadeLabel);
    addAndMakeVisible(mixSlider);
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(envelopeComponent);
    
    //    auto* s = new juce::Slider;
    //
    //    addAndMakeVisible(s);
    //
    //    s->setRange (0.0, 100.0, 0.1);
    //    s->setPopupMenuEnabled (true);
    //    s->setValue (juce::Random::getSystemRandom().nextDouble() * 100.0, juce::dontSendNotification);
    //
    //    s->setSliderStyle (juce::Slider::Rotary);
    //    s->setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, false);
    //    s->setTextBoxStyle (juce::Slider::TextBoxRight, false, 70, 20);
    //    //horizonalSliderArea.removeFromTop (15);
    //    //s->setBounds (horizonalSliderArea.removeFromTop (70));
    //    s->setBounds(0, 0, 20, 20);
    //    s->setTextValueSuffix (" mm");
    
    setSize (400, 800);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    //    g.setColour (juce::Colours::white);
    //    g.setFont (15.0f);
    //    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void NewProjectAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto border = 4;
    
    auto area = getLocalBounds();
    
    auto upperKnobHeight = area.getHeight() / 4;
    auto upperKnobArea = area.removeFromTop (upperKnobHeight);
    auto upperKnobAreaHalfWidth = upperKnobArea.getWidth()/2;
    auto pitchKnobArea = upperKnobArea.removeFromLeft(upperKnobAreaHalfWidth);
    auto formantKnobArea = upperKnobArea.removeFromRight(upperKnobAreaHalfWidth);
    
    auto knobWidth = pitchKnobArea.getWidth()/1.25;
    //    auto knobHeight = area.getHeight() / 4;
    pitchKnob.setBounds (pitchKnobArea.removeFromRight (knobWidth).reduced (border));
    pitchLabel.setBounds(pitchKnob.getBounds());
    //    auto formantKnobArea = area.removeFromTop (area.getHeight() / 4);
    formantKnob.setBounds (formantKnobArea.removeFromLeft (knobWidth).reduced (border));
    formantLabel.setBounds(formantKnob.getBounds());
    
    //    linkButton.setCentrePosition(upperKnobAreaHalfWidth, upperKnobHeight);
    linkButton.setBounds(area.removeFromTop(10));
    linkButton.setSize(20, 20);
    linkButton.changeWidthToFitText();
    linkButton.setCentrePosition(area.getWidth()/2, upperKnobArea.getHeight());
    
    area.removeFromTop(30);
    xfadeLabel.setBounds(area.removeFromTop(30));
//    xfadeLabel.setBounds(area.removeFromTop(40));
    xfadeSlider.setBounds(area.removeFromTop(50).reduced(10));
    
    auto newArea = area.removeFromTop(100);
    auto mixerArea = newArea.removeFromRight(area.getWidth()/2);
    mixSlider.setBounds(mixerArea.removeFromLeft(60));

    auto envelopeArea = newArea.removeFromLeft(area.getWidth()/2);
    attackSlider.setBounds(envelopeArea.removeFromTop(30));
    releaseSlider.setBounds(envelopeArea.removeFromTop(30));

    envelopeComponent.setBounds(envelopeArea.removeFromTop(50));
    envelopeComponent.setSize(area.getWidth()/2, area.getWidth()/2);
    
//    mixSlider.setSize(20, 100);
//    mixSlider.setCentrePosition(area.getWidth()/2, mixSlider.getBounds().getHeight()/2);

//    mixSlider.centreWithSize(20, 100);
    
    //    linkButton.setSize(20, 20);
    //    linkButton.centreWithSize(20, 20);
    //    auto buttonHeight = 30;
    
    //    button1.setBounds (area.removeFromTop (buttonHeight).reduced (border));
    //    button2.setBounds (area.removeFromTop (buttonHeight).reduced (border));
}

//==============================================================================
juce::AudioProcessorParameter* NewProjectAudioProcessorEditor::getParameter (const juce::String& paramId)
{
    if (auto* audioProcessor = getAudioProcessor())
    {
        auto& params = audioProcessor->getParameters();
        
        for (auto p : params)
        {
            if (auto* param = dynamic_cast<juce::AudioProcessorParameterWithID*> (p))
            {
                if (param->paramID == paramId)
                    return param;
            }
        }
    }
    
    return nullptr;
}

//==============================================================================
float NewProjectAudioProcessorEditor::getParameterValue (const juce::String& paramId)
{
    if (auto* param = getParameter (paramId))
        return param->getValue();
    
    return 0.0f;
}

void NewProjectAudioProcessorEditor::setParameterValue (const juce::String& paramId, float value)
{
    if (auto* param = getParameter (paramId))
        param->setValueNotifyingHost (value);
}


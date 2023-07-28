//
//  EnvelopeEditor.h
//  NewProject - Shared Code
//
//  Created by Eugene Clewlow on 7/24/23.
//

#ifndef EnvelopeEditor_h
#define EnvelopeEditor_h

#include <JuceHeader.h>


class EnvelopeEditor  : public juce::Component, juce::Component::MouseListener {
public:
    typedef enum {
        kNoSelection = 0,
        kDelaySelected = 1,
        kCurveSelected = 2,
        kAttackSelected = 3,
    } SelectionState;
    
    typedef enum {
        kNoHover = 0,
        kDelayHovered = 1,
        kCurveHovered = 2,
        kAttackHovered = 3,
    } HoverState;
    
    
    EnvelopeEditor()
    {
        mSelectionState = kNoSelection;
        mHoverState = kNoHover;
        setSize (400, 400);
        curveLastMouseDownPosX=0.0;
        curveLastMouseDownPosY=0.0;
    }
    
    void mouseDown (const juce::MouseEvent& e) override
    {
        float attackPointX = getWidth() * (1.0 / attack + delay);
        float attackPointY = 5.0;
        float delayPointX = getWidth() * delay;
        float delayPointY = getHeight() - 5.0;
        float curvePointX = (delay + (1 / (2 * attack)))*getWidth();
        float curvePointY = (1 - exp_func(delay + (1 / (2 * attack)), delay, attack, curve))*(getHeight()-10);

        float pointX = e.getPosition().x;
        float pointY = e.getPosition().y;
        
        float d1 = sqrtf(pow(attackPointX - pointX, 2) + pow(attackPointY - pointY, 2));
        float d2 = sqrtf(pow(delayPointX - pointX, 2) + pow(delayPointY - pointY, 2));
        float d3 = sqrtf(pow(curvePointX - pointX, 2) + pow(curvePointY - pointY, 2));

        if(isLowest(d1, d2, d3)) {
            mSelectionState = kAttackSelected;
        } else if(isLowest(d2, d1, d3)) {
            mSelectionState = kDelaySelected;
        } else if(isLowest(d3, d2, d1)) {
            mSelectionState = kCurveSelected;
            curveLastMouseDownPosX = pointX;
            curveLastMouseDownPosY = pointY;
        } else {
            mSelectionState = kNoSelection;
        }
        this->repaint();
    }
    
    void mouseUp   (const juce::MouseEvent& e) override
    {
        mSelectionState = kNoSelection;
    }

    void mouseExit   (const juce::MouseEvent& e) override
    {
        mHoverState = kNoHover;
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if(mSelectionState == kAttackSelected)
        {
            float x = fmin(e.getPosition().x, getWidth());
            
            float q = (x / getWidth() - delay);
            
            if(q <= 0) {
                attack = 10.0;
            } else {
                attack = 1.0 / (x / getWidth() - delay);
                
                //z has to be greater than 1 / (1-d)
                if(attack <= 1 / (1-delay)) {
                    attack = 1 / (1-delay);
                }
            }
            attack = clamp(attack, 1.0, 10.0);
            
            onAttackChange();
            
            this->repaint();
        }
        else if(mSelectionState == kDelaySelected)
        {
            float x = fmin(e.getPosition().x, getWidth());
            
            delay = x / getWidth();
            
            if(delay < 0) {
                delay = 0;
            }
            
            // d has to be less than 1 - 1/z
            if(delay > 1 - 1 / attack)
            {
                delay = 1 - 1 / attack;
            }
            
            onDelayChange();
            
            this->repaint();
        }
        else if(mSelectionState == kCurveSelected)
        {
            float pointX = e.getPosition().x;
            float pointY = e.getPosition().y;

            float q = pointY / (getHeight()-10);
            if(q < 0.5) {
                curve = 1.0;
            } else {
                curve = (1 - q) * 2;
            }
            
            curve = clamp(curve, 0.001, 0.999);
            onCurveChange();
            
            this->repaint();
        }
    }
    
    bool isLowest(float d1, float d2, float d3) {
        float lowest = fmin(fmin(d1, d2), d3);
        return d1 == lowest;
    }
    
    void mouseMove (const juce::MouseEvent& e) override
    {
        float attackPointX = getWidth() * (1.0 / attack + delay);
        float attackPointY = 5.0;
        float delayPointX = getWidth() * delay;
        float delayPointY = getHeight() - 5.0;
        float curvePointX = (delay + (1 / (2 * attack)))*getWidth();
        float curvePointY = (1 - exp_func(delay + (1 / (2 * attack)), delay, attack, curve))*(getHeight()-10);

        float pointX = e.getPosition().x;
        float pointY = e.getPosition().y;
        
        float d1 = sqrtf(pow(attackPointX - pointX, 2) + pow(attackPointY - pointY, 2));
        float d2 = sqrtf(pow(delayPointX - pointX, 2) + pow(delayPointY - pointY, 2));
        float d3 = sqrtf(pow(curvePointX - pointX, 2) + pow(curvePointY - pointY, 2));

        if(isLowest(d1, d2, d3)) {
            mHoverState = kAttackHovered;
        } else if(isLowest(d2, d1, d3)) {
            mHoverState = kDelayHovered;
        } else if(isLowest(d3, d2, d1)) {
            mHoverState = kCurveHovered;
        } else {
            mHoverState = kNoHover;
        }
//        mHoverState = kCurveHovered;
        this->repaint();
    }
    
    
    float exp_func(float x, float d, float z, float c) {
        float value;
        value = 1 - (pow(c, 1 + z * (d - x)) - 1) / (c-1);
        if(value > 1.0) {
            value = 1.0;
        }
        if(value < 0) {
            value = 0.0f;
        }
        return value;
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        
        float d = delay; // 0 to 1
        float z = attack; // attack - range 1 to 100
        // d* z has to be 1
        // z has to be greater than 1 / (1-d)
        // d has to be less than 1 - 1/z
        
        float c = curve;// c is the curve 0...1 exclusive!
        
        float x0=0.0, y0=getHeight() - 5.0;
        float slopeX0 = 0.0, slopeY0 = 0.0;
        g.setColour(juce::Colours::blue);
        float precision = 100.0;
        
        auto blueHue = juce::Colour::fromRGB(3, 141, 255).getHue();
        
        float slope = attack;
        float curveSlopeMatch = 1000;
        float curveSlopeMinIndex = 0.0;
        
        juce::Path p;
        p.startNewSubPath (0.0, getHeight() - 5.0);
                
        for(int i = 0; i < precision; i++) {
            float x, y;
            
            x = (float)i / (float)precision;
            
            if(x <= delay && mHoverState == kDelayHovered)
            {
                g.setColour(juce::Colours::lightblue);
            }
            else if(x > delay + 1 / attack && mHoverState == kAttackHovered)
            {
                g.setColour(juce::Colours::lightblue);
            }
            else if(x > delay && x <= delay + 1 / attack && mHoverState == kCurveHovered)
            {
                g.setColour(juce::Colours::lightblue);
            }
            else
            {
                auto finalColor = juce::Colour::fromHSV (blueHue, 0.5 + 0.5 * (1-x), 0.7 + 0.3 * x, 1.0f);

                g.setColour(finalColor);
            }

            
            if(attack > 0) {
                y = exp_func(x, d, z, c);
            }
            else {
                y = 1.0;
            }
                        
            float x1 = x * getWidth();
            float y1 = (1-y) * (getHeight() - 10) + 5;
//            g.drawLine(x0, y0, x1, y1, 4.0);
            p.lineTo(x1, y1);
            
            float currentSlope = (y-slopeY0) / (x-slopeX0);
            
            if(fabs(slope - currentSlope) < curveSlopeMatch) {
                curveSlopeMinIndex = i;
                curveSlopeMatch = fabs(slope - currentSlope);
            }
            
            x0 = x1;
            y0 = y1;
            
            slopeX0 = x;
            slopeY0 = y;
        }
        
        g.strokePath(p, juce::PathStrokeType(4.0));
        
        float attackPoint =  1.0 / attack + delay;
        float delayPoint = delay;
        g.setColour(juce::Colours::lightblue);
//        curveSlopeMinIndex = 50;
        float curvePointX = curveSlopeMinIndex / precision;
//        float curvePointX = delay + (1 / (2 * attack));
        float curvePointY = 1 - exp_func(curveSlopeMinIndex / precision, d, z, c);
        
        if(curve >= 0.8) {
            curvePointX = delay + (1 / (2 * attack));
            curvePointY = 1 - exp_func(curvePointX, d, z, c);
        }
        
        float ellipseWidth = 5.0;
        float ellipseHeight = 5.0;
        
        if(mHoverState == kAttackHovered)
        {
            g.drawEllipse(attackPoint * getWidth() - ellipseWidth / 2, 5.0 - ellipseHeight / 2, 5.0, 5.0, 5.0);
        } else if (mHoverState == kDelayHovered)
        {
            g.drawEllipse(delayPoint * getWidth() - ellipseWidth / 2, getHeight() - 5 - ellipseHeight / 2, 5.0, 5.0, 5.0);
        } else if (mHoverState == kCurveHovered)
        {
            g.drawEllipse(curvePointX * getWidth() - ellipseWidth / 2, curvePointY * (getHeight() - 10) + 5 - ellipseHeight / 2, 5.0, 5.0, 5.0);
        }
    }
    
    
    void resized() override
    {
        //        scene.setBounds (getLocalBounds());
    }
    
    void setAttack(float newValue) {
        attack = newValue;
    }

    void setDelay(float newValue) {
        delay = newValue;
    }

    void setCurve(float newValue) {
        newValue = clamp(newValue, 0.001, 0.999);
        curve = newValue;
    }
    
    float getAttack() {
        return attack;
    }

    float getDelay() {
        return delay;
    }

    float getCurve() {
        return curve;
    }

    
    std::function<void()> onAttackChange;
    std::function<void()> onDelayChange;
    std::function<void()> onCurveChange;
    
    void setAttackChangeListener(std::function<void()> func) {
        onAttackChange = func;
    }

    void setDelayChangeListener(std::function<void()> func) {
        onDelayChange = func;
    }

    void setCurveChangeListener(std::function<void()> func) {
        onCurveChange = func;
    }


private:
    float attack;
    float delay;
    float curve;
    float curveLastMouseDownPosX;
    float curveLastMouseDownPosY;
    SelectionState mSelectionState;
    HoverState mHoverState;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeEditor)
};

class EnvelopeComponent  : public juce::Component, juce::Component::MouseListener {
public:
    typedef enum {
        kNoSelection = 0,
        kPitchSelected = 1,
        kFormantSelected = 2,
    } SelectionState;
    
    typedef enum {
        kNoHover = 0,
        kPitchHovered = 1,
        kFormantHovered = 2,
    } HoverState;

    EnvelopeComponent()
    {
        addAndMakeVisible(envelopeEditor, 0);
        setSize (400, 400);
        setInterceptsMouseClicks(true, true);
    }
    
    void mouseDown (const juce::MouseEvent& e) override
    {
        float pointX = e.getPosition().x;
        float pointY = e.getPosition().y;

        float dx = pointX - getWidth() / 2;
        float dy = getHeight() / 2 - pointY;
        
        float d1 = sqrtf(pow(dx, 2) + pow(dy, 2));
        
        if(d1 > getHeight() / 2 - 20.0f && dy > 0) {
            mSelectionState = kPitchSelected;
        }
        else if(d1 > getHeight() / 2 - 20.0f && dy < 0) {
            mSelectionState = kFormantSelected;
        }
        else
        {
            mSelectionState = kNoSelection;
        }
        this->repaint();
        
    }
    
    void mouseUp   (const juce::MouseEvent& e) override
    {
        mSelectionState = kNoSelection;
    }
    
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if(mSelectionState == kPitchSelected)
        {
            float x = clamp(e.getPosition().x, 0.0f, getWidth());
            pitch_envelope_amount = x / getWidth();
            onPitchEnvelopeAmountChange();
            
            this->repaint();
        }
        else if(mSelectionState == kFormantSelected)
        {
            float x = clamp(e.getPosition().x, 0.0f, getWidth());
            formant_envelope_amount = x / getWidth();
            onFormantEnvelopeAmountChange();

            this->repaint();
        }
    }
    
    void mouseExit (const juce::MouseEvent& e) override
    {
        mHoverState = kNoHover;
    }
    
    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        float pointX = e.getPosition().x;
        float pointY = e.getPosition().y;

        float dx = pointX - getWidth() / 2;
        float dy = getHeight() / 2 - pointY;
        
        float d1 = sqrtf(pow(dx, 2) + pow(dy, 2));
        
        if(d1 > getHeight() / 2 - 20.0f && dy > 0) {
            pitch_envelope_amount = 0.5f;
            onPitchEnvelopeAmountChange();
        }
        else if(d1 > getHeight() / 2 - 20.0f && dy < 0) {
            formant_envelope_amount = 0.5f;
            onFormantEnvelopeAmountChange();
        }
        this->repaint();
    }
    
    void mouseMove (const juce::MouseEvent& e) override
    {
        float pointX = e.getPosition().x;
        float pointY = e.getPosition().y;

        float dx = pointX - getWidth() / 2;
        float dy = getHeight() / 2 - pointY;
        
        float d1 = sqrtf(pow(dx, 2) + pow(dy, 2));
        
        if(d1 > getHeight() / 2 - 20.0f && dy > 0) {
            mHoverState = kPitchHovered;
        }
        else if(d1 > getHeight() / 2 - 20.0f && dy < 0) {
            mHoverState = kFormantHovered;
        }
        else
        {
            mHoverState = kNoHover;
        }
        this->repaint();
    }
    
    static double radiansToDegrees (double rads) noexcept { return (180.0 / juce::MathConstants<double>::pi) * rads; }
    static double degreesToRadians (double degs) noexcept { return (juce::MathConstants<double>::pi / 180.0) * (90.0f-degs); }

    static float radiansToDegrees (float rads) noexcept { return (180.0f / juce::MathConstants<float>::pi) * rads; }
    static float degreesToRadians (float degs) noexcept { return (juce::MathConstants<float>::pi / 180.0f) * (90.0f-degs); }
    static float degreesToRadiansEllipse (float degs) noexcept { return (juce::MathConstants<float>::pi / 180.0f) * (0.0f+degs);}
    static float radiansToDegreesEllipse (float rads) noexcept { return (180.0f / juce::MathConstants<float>::pi) * rads; }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
//        g.setColour (juce::Colours::white);
//        g.setFont (15.0f);
//        g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
        
        juce::Path p;
        
        juce::PathStrokeType pathStrokeType = juce::PathStrokeType(4.0);
        pathStrokeType.setEndStyle(juce::PathStrokeType::EndCapStyle::rounded);

        float fromRadians;
        float toRadians;

        float x = 0;
        float y = 0;

        float ellipseSize = 8;
        float ellipseSize2 = ellipseSize * 2;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        p = juce::Path();
        // top grey
        fromRadians = degreesToRadians(0.0f+10.0f);
        toRadians = degreesToRadians(180.0f-10.0f);
        
        
        p.addArc(x+ellipseSize2/2, y+ellipseSize2/2, getWidth()-ellipseSize2, getHeight()-ellipseSize2, fromRadians, toRadians, true);
        g.setColour(juce::Colours::grey);
        g.strokePath(p, pathStrokeType);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        p = juce::Path();
        // top green
        fromRadians = degreesToRadians(90.0f);
        toRadians = degreesToRadians(160.0f * (1-pitch_envelope_amount) + 10.0f);
        
        
        p.addArc(x+ellipseSize2/2, y+ellipseSize2/2, getWidth()-ellipseSize2, getHeight()-ellipseSize2, fromRadians, toRadians, true);
        g.setColour(juce::Colours::greenyellow);
        g.strokePath(p, pathStrokeType);


        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        p = juce::Path();
        // bottom grey
        fromRadians = degreesToRadians(180.0f+10.0f);
        toRadians = degreesToRadians(360.0f-10.0f);

        p.addArc(x+ellipseSize2/2, y+ellipseSize2/2, getWidth()-ellipseSize2, getHeight()-ellipseSize2, fromRadians, toRadians, true);
        g.setColour(juce::Colours::grey);
        g.strokePath(p, pathStrokeType);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        p = juce::Path();
        // bottom red
        fromRadians = degreesToRadians(270.0f);
        toRadians = degreesToRadians(360.0f-(160.0f * (1-formant_envelope_amount) + 10.0f));

        p.addArc(x+ellipseSize2/2, y+ellipseSize2/2, getWidth()-ellipseSize2, getHeight()-ellipseSize2, fromRadians, toRadians, true);
        g.setColour(juce::Colours::red);
        g.strokePath(p, pathStrokeType);
        
        

        float rx, ry, dx, dy;
        rx = (getWidth() - ellipseSize2) / 2;
        ry = (getHeight() - ellipseSize2) / 2;
        if(mHoverState == kPitchHovered || mSelectionState == kPitchSelected) {
            g.setColour(juce::Colours::greenyellow);

            dx = rx * cosf(degreesToRadiansEllipse(160.0f * (1-pitch_envelope_amount) + 10.0f));
            dy = ry * sinf(degreesToRadiansEllipse(160.0f * (1-pitch_envelope_amount) + 10.0f));
            g.drawEllipse(getWidth()/2 + dx - ellipseSize/2, getHeight()/2 - dy - ellipseSize / 2, ellipseSize, ellipseSize, ellipseSize);
        }
        else if(mHoverState == kFormantHovered || mSelectionState == kFormantSelected) {
            g.setColour(juce::Colours::red);

            dx = rx * cosf(degreesToRadiansEllipse(0.0f - 160.0f * (1-formant_envelope_amount) - 10.0f));
            dy = ry * sinf(degreesToRadiansEllipse(0.0f - 160.0f * (1-formant_envelope_amount) - 10.0f));
            g.drawEllipse(getWidth()/2 + dx - ellipseSize / 2, getHeight()/2 - dy - ellipseSize / 2, ellipseSize, ellipseSize, ellipseSize);
        }
    }
    
    
    void resized() override
    {
//        pitchSlider.setBounds(0, 0, getWidth(), getHeight());
//        envelopeEditor.setBounds(0, 0, getWidth(), getHeight());
        envelopeEditor.centreWithSize(getWidth() / 1.5, getHeight()/3);
    }
    
    std::function<void()> onPitchEnvelopeAmountChange;
    std::function<void()> onFormantEnvelopeAmountChange;

    void setFormantEnvelopeAmountChangeListener(std::function<void()> func) {
        onFormantEnvelopeAmountChange = func;
    }

    void setPitchEnvelopeAmountChangeListener(std::function<void()> func) {
        onPitchEnvelopeAmountChange = func;
    }

    void setAttackChangeListener(std::function<void()> func) {
        envelopeEditor.setAttackChangeListener(func);
    }

    void setDelayChangeListener(std::function<void()> func) {
        envelopeEditor.setDelayChangeListener(func);
    }

    void setCurveChangeListener(std::function<void()> func) {
        envelopeEditor.setCurveChangeListener(func);
    }

    void setAttack (double newValue, juce::NotificationType notification)
    {
        envelopeEditor.setAttack(newValue);
    }

    void setDelay (double newValue, juce::NotificationType notification)
    {
        envelopeEditor.setDelay(newValue);
    }

    void setCurve (double newValue, juce::NotificationType notification)
    {
        envelopeEditor.setCurve(newValue);
    }

    void setPitchEnvelopeAmount (double newValue, juce::NotificationType notification)
    {
        pitch_envelope_amount = newValue;
    }

    void setFormantEnvelopeAmount (double newValue, juce::NotificationType notification)
    {
        formant_envelope_amount = newValue;
    }
    
    float getAttack() {
        return envelopeEditor.getAttack();
    }

    float getDelay() {
        return envelopeEditor.getDelay();
    }

    float getCurve() {
        return envelopeEditor.getCurve();
    }

    float getPitchEnvelopeAmount() {
        return pitch_envelope_amount;
    }

    float getFormantEnvelopeAmount() {
        return formant_envelope_amount;
    }

    
private:
    float pitch_envelope_amount;
    float formant_envelope_amount;
    EnvelopeEditor envelopeEditor;
    SelectionState mSelectionState;
    HoverState mHoverState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeComponent)
};

#endif /* EnvelopeEditor_h */

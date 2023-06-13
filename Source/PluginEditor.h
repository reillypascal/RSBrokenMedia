/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//look and feel of sliders/labels
class GrayBlueLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GrayBlueLookAndFeel()
    {
        // colors
        // sliders
        setColour(juce::Slider::thumbColourId, juce::Colours::aliceblue);
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::lightsteelblue);
        setColour(juce::Slider::trackColourId, juce::Colours::slategrey);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::aliceblue);
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::aliceblue);
        // labels
        setColour(juce::Label::textColourId, juce::Colours::aliceblue);
    }
};

//==============================================================================
/**
*/
class RSBrokenMediaAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    RSBrokenMediaAudioProcessorEditor (RSBrokenMediaAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~RSBrokenMediaAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    juce::Label clockLabel;
    juce::Label analogFXLabel;
    juce::Label digitalFXLabel;
    juce::Label lofiFXLabel;
    
    juce::Slider clockSlider;
    juce::Slider analogFXSlider;
    juce::Slider digitalFXSlider;
    juce::Slider lofiFXSlider;
    
    std::unique_ptr<SliderAttachment> clockAttachment;
    std::unique_ptr<SliderAttachment> analogFXAttachment;
    std::unique_ptr<SliderAttachment> digitalFXAttachment;
    std::unique_ptr<SliderAttachment> lofiFXAttachment;
    
    const int textBoxWidth = 70;
    const int textBoxHeight = 25;
    GrayBlueLookAndFeel grayBlueLookAndFeel;
    
    RSBrokenMediaAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RSBrokenMediaAudioProcessorEditor)
};

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
        // colours
        // sliders
        setColour(juce::Slider::thumbColourId, juce::Colours::aliceblue);
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::lightsteelblue);
        //setColour(juce::Slider::trackColourId, juce::Colours::slategrey);
        setColour(juce::Slider::trackColourId, juce::Colours::lightslategrey);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::aliceblue);
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::aliceblue);
        // labels
        setColour(juce::Label::textColourId, juce::Colours::aliceblue);
        // menus
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xAA88CC)); // doesn't read as actual colour?
        setColour(juce::ComboBox::buttonColourId, juce::Colours::slategrey);
        setColour(juce::ComboBox::textColourId, juce::Colours::aliceblue);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::aliceblue);
        setColour(juce::ComboBox::arrowColourId, juce::Colours::aliceblue);
        
        // fonts
        setDefaultSansSerifTypeface(juce::LookAndFeel::getTypefaceForFont(juce::Font("Verdana", 18.0f, juce::Font::plain)));
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
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    juce::Label analogFXLabel;
    juce::Label digitalFXLabel;
    juce::Label lofiFXLabel;
    
    juce::Label clockLabel;
    juce::Label bufferLengthLabel;
    juce::Label repeatsLabel;
    juce::Label dryWetMixLabel;
    
    juce::Slider analogFXSlider;
    juce::Slider digitalFXSlider;
    juce::Slider lofiFXSlider;
    
    juce::Slider clockSlider;
    juce::Slider bufferLengthSlider;
    juce::Slider repeatsSlider;
    juce::Slider dryWetMixSlider;
    
    juce::ComboBox codecModeMenu;
    juce::ComboBox downsamplingMenu;
    enum codecModes
    {
        normal = 1,
        mulaw,
        gsm610
    };
    enum downsamplingModes
    {
        none = 1,
        x2,
        x4,
        x8,
        x16
    };
    
    std::unique_ptr<SliderAttachment> analogFXAttachment;
    std::unique_ptr<SliderAttachment> digitalFXAttachment;
    std::unique_ptr<SliderAttachment> lofiFXAttachment;
    
    std::unique_ptr<SliderAttachment> clockAttachment;
    std::unique_ptr<SliderAttachment> bufferLengthAttachment;
    std::unique_ptr<SliderAttachment> repeatsAttachment;
    std::unique_ptr<SliderAttachment> dryWetMixAttachment;
    
    std::unique_ptr<ComboBoxAttachment> codecModeMenuAttachment;
    std::unique_ptr<ComboBoxAttachment> downsamplingMenuAttachment;
    
    const int textBoxWidth = 70;
    const int textBoxHeight = 25;
    GrayBlueLookAndFeel grayBlueLookAndFeel;
    
    RSBrokenMediaAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RSBrokenMediaAudioProcessorEditor)
};

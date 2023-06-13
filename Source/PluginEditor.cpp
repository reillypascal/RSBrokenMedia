/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RSBrokenMediaAudioProcessorEditor::RSBrokenMediaAudioProcessorEditor (RSBrokenMediaAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (&p), valueTreeState(vts), audioProcessor (p)
{
    // labels row 1
    clockLabel.setText("Clock Rate", juce::dontSendNotification);
    clockLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(clockLabel);
    
    analogFXLabel.setText("Analog FX", juce::dontSendNotification);
    analogFXLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(analogFXLabel);
    
    digitalFXLabel.setText("Digital FX", juce::dontSendNotification);
    digitalFXLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(digitalFXLabel);
    
    lofiFXLabel.setText("Lo-Fi FX", juce::dontSendNotification);
    lofiFXLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lofiFXLabel);
    
    // labels row 2
    dryWetMixLabel.setText("Dry/Wet Mix", juce::dontSendNotification);
    dryWetMixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(dryWetMixLabel);
    
    // sliders row 1
    clockSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    clockSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(clockSlider);
    clockAttachment.reset(new SliderAttachment(valueTreeState, "clockSpeed", clockSlider));
    
    analogFXSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    analogFXSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(analogFXSlider);
    analogFXAttachment.reset(new SliderAttachment(valueTreeState, "analogFX", analogFXSlider));
    
    digitalFXSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    digitalFXSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(digitalFXSlider);
    digitalFXAttachment.reset(new SliderAttachment(valueTreeState, "digitalFX", digitalFXSlider));
    
    lofiFXSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    lofiFXSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(lofiFXSlider);
    lofiFXAttachment.reset(new SliderAttachment(valueTreeState, "lofiFX", lofiFXSlider));
    
    // sliders row 2
    dryWetMixSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    dryWetMixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(dryWetMixSlider);
    dryWetMixAttachment.reset(new SliderAttachment(valueTreeState, "dryWetMix", dryWetMixSlider));
    
    getLookAndFeel().setDefaultLookAndFeel(&grayBlueLookAndFeel);
    
    setSize (800, 525);
}

RSBrokenMediaAudioProcessorEditor::~RSBrokenMediaAudioProcessorEditor()
{
}

//==============================================================================
void RSBrokenMediaAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB(12, 16, 20)); //was 0x221144

    g.setColour (juce::Colours::aliceblue);
    g.setFont (32.0f);
    g.drawFittedText ("RS Broken Media", 0, 10, getWidth(), 50, juce::Justification::centred, 1);
}

void RSBrokenMediaAudioProcessorEditor::resized()
{
    const int xBorder = 30;
    const int yBorderTop = 75;
    const int yBorderBottom = 50;
    const int rowSpacer = 45;
    
    //const int menuWidth = 200;
    //const int menuHeight = 20;
    const int sliderWidth1 = (getWidth() - (2 * xBorder)) / 4;
    const int sliderWidth2 = (getWidth() - (2 * xBorder)) / 5;
    const int sliderHeight1 = (getHeight() - (yBorderTop + yBorderBottom) - rowSpacer) / 2;
    const int sliderHeight2 = sliderHeight1 * 0.8;
    const int textLabelWidth = 150;
    const int textLabelHeight = 20;
    const int textLabelSpacer = 7;
    
    // row 1 sliders
    clockSlider.setBounds(xBorder, yBorderTop, sliderWidth1, sliderHeight1);
    analogFXSlider.setBounds(xBorder + sliderWidth1, yBorderTop, sliderWidth1, sliderHeight1);
    digitalFXSlider.setBounds(xBorder + (2 * sliderWidth1), yBorderTop, sliderWidth1, sliderHeight1);
    lofiFXSlider.setBounds(xBorder + (3 * sliderWidth1), yBorderTop, sliderWidth1, sliderHeight1);
    
    // row 2 sliders
    dryWetMixSlider.setBounds(xBorder + (sliderWidth2 * 4), yBorderTop + sliderHeight1 + rowSpacer, sliderWidth2, sliderHeight2);
    
    // row 1 labels
    clockLabel.setBounds(xBorder + ((sliderWidth1 / 2) - (textLabelWidth / 2)), yBorderTop + sliderHeight1 + textLabelSpacer, textLabelWidth, textLabelHeight);
    analogFXLabel.setBounds(xBorder + sliderWidth1 + ((sliderWidth1 / 2) - (textLabelWidth / 2)), yBorderTop + sliderHeight1 + textLabelSpacer, textLabelWidth, textLabelHeight);
    digitalFXLabel.setBounds(xBorder + (sliderWidth1 * 2) + ((sliderWidth1 / 2) - (textLabelWidth / 2)), yBorderTop + sliderHeight1 + textLabelSpacer, textLabelWidth, textLabelHeight);
    lofiFXLabel.setBounds(xBorder + (sliderWidth1 * 3) + ((sliderWidth1 / 2) - (textLabelWidth / 2)), yBorderTop + sliderHeight1 + textLabelSpacer, textLabelWidth, textLabelHeight);
    
    // row 2 labels
    dryWetMixLabel.setBounds(xBorder + (sliderWidth2 * 4) + (sliderWidth2 / 2) - (textLabelWidth / 2), yBorderTop + sliderHeight1 + sliderHeight2 + rowSpacer + textLabelSpacer, textLabelWidth, textLabelHeight);
}

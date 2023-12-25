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
    analogFXLabel.setText("Analog FX", juce::dontSendNotification);
    analogFXLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(analogFXLabel);
    
    digitalFXLabel.setText("Digital FX", juce::dontSendNotification);
    digitalFXLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(digitalFXLabel);
    
    lofiFXLabel.setText("Distortion FX", juce::dontSendNotification);
    lofiFXLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lofiFXLabel);
    
    // labels row 2
    clockLabel.setText("Clock Rate", juce::dontSendNotification);
    clockLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(clockLabel);
    
    bufferLengthLabel.setText("Buffer Length", juce::dontSendNotification);
    bufferLengthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bufferLengthLabel);
    
    repeatsLabel.setText("Repeats", juce::dontSendNotification);
    repeatsLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(repeatsLabel);
    
    dryWetMixLabel.setText("Dry/Wet Mix", juce::dontSendNotification);
    dryWetMixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(dryWetMixLabel);
    
    // menu labels
    codecModeLabel.setText("Codec:", juce::dontSendNotification);
    codecModeLabel.setJustificationType(juce::Justification::right);
    addAndMakeVisible(codecModeLabel);
    
    downsamplingLabel.setText("Downsamp:", juce::dontSendNotification);
    downsamplingLabel.setJustificationType(juce::Justification::right);
    addAndMakeVisible(downsamplingLabel);
    
    // sliders row 1
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
    clockSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    clockSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(clockSlider);
    clockAttachment.reset(new SliderAttachment(valueTreeState, "clockSpeed", clockSlider));
    
    // button
    clockModeButton.setButtonText("Sync");
    clockModeButton.setToggleable(true);
    clockModeButton.setClickingTogglesState(true);
    clockModeButton.onClick = [&]
    {
        bool buttonState = clockModeButton.getToggleState();
        if (buttonState == true)
        {
            clockAttachment.reset(new SliderAttachment(valueTreeState, "clockSpeedNote", clockSlider));
            p.setUseDawClock(buttonState);
        }
        else
        {
            clockAttachment.reset(new SliderAttachment(valueTreeState, "clockSpeed", clockSlider));
            p.setUseDawClock(buttonState);
        }
    };
    addAndMakeVisible(clockModeButton);
    clockModeAttachment.reset(new ButtonAttachment(valueTreeState, "clockMode", clockModeButton));
    
    // sliders row 2 (cont.)
    bufferLengthSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    bufferLengthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(bufferLengthSlider);
    bufferLengthAttachment.reset(new SliderAttachment(valueTreeState, "bufferLength", bufferLengthSlider));
    
    repeatsSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    repeatsSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(repeatsSlider);
    repeatsAttachment.reset(new SliderAttachment(valueTreeState, "repeats", repeatsSlider));
    
    dryWetMixSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    dryWetMixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(dryWetMixSlider);
    dryWetMixAttachment.reset(new SliderAttachment(valueTreeState, "dryWetMix", dryWetMixSlider));
    
    // menus
    addAndMakeVisible(codecModeMenu);
    using enum CodecModes;
    codecModeMenu.addItem("None", static_cast<int>(normal));
    codecModeMenu.addItem("Mu-Law", static_cast<int>(mulaw));
    codecModeMenu.addItem("GSM 06.10", static_cast<int>(gsm610));
    codecModeMenu.setSelectedId(static_cast<int>(normal));
    codecModeMenu.setTextWhenNothingSelected("Codec:");
    codecModeMenu.setJustificationType(juce::Justification::centred);
    codecModeMenuAttachment.reset(new ComboBoxAttachment(valueTreeState, "codec", codecModeMenu));
    
    addAndMakeVisible(downsamplingMenu);
//    using enum DownsamplingModes;
    downsamplingMenu.addItem("None", 1);
    downsamplingMenu.addItem("x2", 2);
    downsamplingMenu.addItem("x3", 3);
    downsamplingMenu.addItem("x4", 4);
    downsamplingMenu.addItem("x5", 5);
    downsamplingMenu.addItem("x6", 6);
    downsamplingMenu.addItem("x7", 7);
    downsamplingMenu.addItem("x8", 8);
    downsamplingMenu.setSelectedId(1);
    downsamplingMenu.setTextWhenNothingSelected("Downsamp:");
    downsamplingMenu.setJustificationType(juce::Justification::centred);
    downsamplingMenuAttachment.reset(new ComboBoxAttachment(valueTreeState, "downsampling", downsamplingMenu));
    
    getLookAndFeel().setDefaultLookAndFeel(&grayBlueLookAndFeel);
        
    setSize (755, 575);
}

RSBrokenMediaAudioProcessorEditor::~RSBrokenMediaAudioProcessorEditor()
{
}

//==============================================================================
void RSBrokenMediaAudioProcessorEditor::paint (juce::Graphics& g)
{
    // background
    // tried: 0x221144; 12, 16, 20; 21, 32, 43; 51, 51, 51 (outlive max patch)
    g.fillAll (juce::Colour::fromRGB(32, 32, 32));

    g.setColour (juce::Colours::aliceblue);
    // Title
    g.setFont (juce::Font("Verdana", 32.0f, juce::Font::plain));
    g.drawFittedText ("RS Broken Media", 25, 10, 350, 45, juce::Justification::left, 1);
    // info
    g.setFont (juce::Font("Verdana", 16.0f, juce::Font::plain));
    g.drawFittedText ("Version 0.3.0\n reillyspitzfaden.netlify.app", getWidth() - 375, 15, 350, 45, juce::Justification::right, 2);
    
    // panels
    // tried 93,107,128 (outlive max patch)
    g.setColour(juce::Colour::fromRGB(68, 81, 96));
    g.fillRoundedRectangle(25, 65, getWidth() - 50, 237, 25);
    g.fillRoundedRectangle(25, 327, getWidth() - 50, 183, 25);
}

void RSBrokenMediaAudioProcessorEditor::resized()
{
    const int xBorder = 30;
    const int yBorderTop = 85;
    const int yBorderBottom = 35;
    const int rowSpacer = 87;
    const int bottomMenuSpacer = 20;
    
    //const int menuWidth = 200;
    const int menuHeight = 20;
    const int sliderWidth1 = (getWidth() - (2 * xBorder)) / 3;
    const int sliderWidth2 = (getWidth() - (2 * xBorder)) / 4;
    const int sliderHeight1 = (getHeight() - yBorderTop - yBorderBottom - rowSpacer - bottomMenuSpacer - menuHeight) / 2;
    const int sliderHeight2 = sliderHeight1 * 0.8;
    const int textLabelWidth = 150;
    const int textLabelHeight = 20;
    const int textLabelSpacer = 7;
    
    // row 1 sliders
    analogFXSlider.setBounds(xBorder,
                             yBorderTop,
                             sliderWidth1,
                             sliderHeight1);
    digitalFXSlider.setBounds(xBorder + sliderWidth1,
                              yBorderTop,
                              sliderWidth1,
                              sliderHeight1);
    lofiFXSlider.setBounds(xBorder + (2 * sliderWidth1),
                           yBorderTop,
                           sliderWidth1,
                           sliderHeight1);
    
    // row 2 sliders
    clockSlider.setBounds(xBorder,
                          yBorderTop + sliderHeight1 + rowSpacer,
                          sliderWidth2,
                          sliderHeight2);
    bufferLengthSlider.setBounds(xBorder + sliderWidth2,
                                 yBorderTop + sliderHeight1 + rowSpacer,
                                 sliderWidth2,
                                 sliderHeight2);
    repeatsSlider.setBounds(xBorder + + (sliderWidth2 * 2),
                            yBorderTop + sliderHeight1 + rowSpacer,
                            sliderWidth2,
                            sliderHeight2);
    dryWetMixSlider.setBounds(xBorder + (sliderWidth2 * 3),
                              yBorderTop + sliderHeight1 + rowSpacer,
                              sliderWidth2,
                              sliderHeight2);
    // button
    clockModeButton.setBounds(xBorder + (sliderWidth2 / 2) + (textBoxWidth / 2) + 12,
                              yBorderTop + sliderHeight1 + rowSpacer + 106,
                              45,
                              textBoxHeight);
    
    // row 1 labels
    analogFXLabel.setBounds(xBorder + ((sliderWidth1 / 2) - (textLabelWidth / 2)),
                            yBorderTop + sliderHeight1 + textLabelSpacer,
                            textLabelWidth,
                            textLabelHeight);
    digitalFXLabel.setBounds(xBorder + sliderWidth1 + ((sliderWidth1 / 2) - (textLabelWidth / 2)),
                             yBorderTop + sliderHeight1 + textLabelSpacer,
                             textLabelWidth,
                             textLabelHeight);
    lofiFXLabel.setBounds(xBorder + (sliderWidth1 * 2) + ((sliderWidth1 / 2) - (textLabelWidth / 2)),
                          yBorderTop + sliderHeight1 + textLabelSpacer,
                          textLabelWidth,
                          textLabelHeight);
    
    // row 2 labels
    clockLabel.setBounds(xBorder + ((sliderWidth2 / 2) - (textLabelWidth / 2)),
                         yBorderTop + sliderHeight1 + sliderHeight2 + rowSpacer + textLabelSpacer,
                         textLabelWidth,
                         textLabelHeight);
    bufferLengthLabel.setBounds(xBorder + sliderWidth2 + ((sliderWidth2 / 2) - (textLabelWidth / 2)),
                                yBorderTop + sliderHeight1 + sliderHeight2 + rowSpacer + textLabelSpacer,
                                textLabelWidth,
                                textLabelHeight);
    repeatsLabel.setBounds(xBorder + (sliderWidth2 * 2) + (sliderWidth2 / 2) - (textLabelWidth / 2),
                           yBorderTop + sliderHeight1 + sliderHeight2 + rowSpacer + textLabelSpacer,
                           textLabelWidth,
                           textLabelHeight);
    dryWetMixLabel.setBounds(xBorder + (sliderWidth2 * 3) + (sliderWidth2 / 2) - (textLabelWidth / 2),
                             yBorderTop + sliderHeight1 + sliderHeight2 + rowSpacer + textLabelSpacer,
                             textLabelWidth,
                             textLabelHeight);
    
    // menus
    codecModeLabel.setBounds(getWidth() - 25 - 435,
                             getHeight() - 25 - menuHeight,
                             75,
                             menuHeight);
    codecModeMenu.setBounds(getWidth() - 25 - 357,
                            getHeight() - 25 - menuHeight,
                            125,
                            menuHeight);
    downsamplingLabel.setBounds(getWidth() - 25 - 223,
                             getHeight() - 25 - menuHeight,
                             95,
                             menuHeight);
    downsamplingMenu.setBounds(getWidth() - 25 - 125,
                            getHeight() - 25 - menuHeight,
                            125,
                            menuHeight);
}

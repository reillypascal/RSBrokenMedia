/*
  ==============================================================================

    Plugin GUI interface

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUIStyles.h"

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
    
    // labels
    juce::Label analogFXLabel;
    juce::Label digitalFXLabel;
    juce::Label lofiFXLabel;
    
    juce::Label clockLabel;
    juce::Label bufferLengthLabel;
    juce::Label repeatsLabel;
    juce::Label dryWetMixLabel;
    
    juce::Label distLabel;
    juce::Label codecModeLabel;
    juce::Label downsamplingLabel;
    
    // sliders
    juce::Slider analogFXSlider;
    juce::Slider digitalFXSlider;
    juce::Slider lofiFXSlider;
    
    juce::Slider clockSlider;
    juce::Slider bufferLengthSlider;
    juce::Slider repeatsSlider;
    juce::Slider dryWetMixSlider;
    
    // button
    juce::TextButton clockModeButton;
    
    // dropdowns
    juce::ComboBox distMenu;
    juce::ComboBox codecModeMenu;
    juce::ComboBox downsamplingMenu;
    
    // attachments
    std::unique_ptr<SliderAttachment> analogFXAttachment;
    std::unique_ptr<SliderAttachment> digitalFXAttachment;
    std::unique_ptr<SliderAttachment> lofiFXAttachment;
    
    std::unique_ptr<SliderAttachment> clockAttachment;
    std::unique_ptr<SliderAttachment> bufferLengthAttachment;
    std::unique_ptr<SliderAttachment> repeatsAttachment;
    std::unique_ptr<SliderAttachment> dryWetMixAttachment;
    
    std::unique_ptr<ButtonAttachment> clockModeAttachment;
    
    std::unique_ptr<ComboBoxAttachment> distMenuAttachment;
    std::unique_ptr<ComboBoxAttachment> codecModeMenuAttachment;
    std::unique_ptr<ComboBoxAttachment> downsamplingMenuAttachment;
    
    // GUI parameters
    GrayBlueLookAndFeel grayBlueLookAndFeel;
    const int textBoxWidth = 70;
    const int textBoxHeight = 25;
    
    RSBrokenMediaAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RSBrokenMediaAudioProcessorEditor)
};

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RSBrokenMediaAudioProcessorEditor::RSBrokenMediaAudioProcessorEditor (RSBrokenMediaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    
    
    setSize (800, 525);
}

RSBrokenMediaAudioProcessorEditor::~RSBrokenMediaAudioProcessorEditor()
{
}

//==============================================================================
void RSBrokenMediaAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0x221144));

    g.setColour (juce::Colours::aliceblue);
    g.setFont (32.0f);
    g.drawFittedText ("RS Broken Media", 0, 10, getWidth(), 50, juce::Justification::centred, 1);
}

void RSBrokenMediaAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

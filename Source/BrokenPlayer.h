/*
  ==============================================================================

    

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CircularBuffer.h"
#include "Modulators.h"
#include "Utilities.h"

class BrokenPlayer : public juce::AudioProcessor
{
public:
    BrokenPlayer();
    
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    void reset() override;
    
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;
    
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override;
    
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    
    //==============================================================================
    float wrap(float a, float b);
    
private:
    int mGranBufferLength = 88200;
    CircularBuffer<float> mCircularBuffer { mGranBufferLength };
    
    std::vector<float> mReadPosition { 0, 0 };
    std::vector<float> mPlaybackRate = { 1.0, 1.0 };
    
    OscillatorParameters lfoParameters;
    SignalGenData<float> lfoOutput;
    LFO<float> lfo;
    Delta<float> deltaObj;
    
    Line<float> leftLine;
    Line<float> rightLine;
    float rampTime = 150;
    
    std::vector<float> tapeBendVals { 0.5, 0.67, 1.0, 1.5, 2.0 };
};

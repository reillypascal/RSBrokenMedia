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
    int mGranBufferLength = 66150;
    CircularBuffer<float> mCircularBuffer { mGranBufferLength };
    
    std::vector<float> mReadPosition { 0, 0 };
    std::vector<float> mPlaybackRate = { 1.0, 1.0 };
    
    OscillatorParameters lfoParameters;
    SignalGenData<float> lfoOutput;
    
    std::vector<Line<float>> tapeSpeedLine { Line<float>(), Line<float>()};
    std::vector<Line<float>> tapeStopLine { Line<float>(), Line<float>()};
    //Line<float> leftLine;
    //Line<float> rightLine;
    //Line<float> leftTapeStop;
    //Line<float> rightTapeStop;
    float rampTime = 6615;
    
    std::vector<float> tapeBendVals { 0.5, 0.67, 1.0, 1.5, 2.0 };
    std::vector<float> tapeDirVals { -1, 1 };
    int clockCounter = 0;
    int clockCycle = 33075;
    
    // parameters
    float clockPeriod = 675;
    float tapeBendProb = 1;
    float tapeRevProb = 0.33;
    float tapeStopProb = 0.25;
};

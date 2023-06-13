/*
  ==============================================================================

    

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CircularBuffer.h"
#include "Modulators.h"
#include "Utilities.h"

//==============================================================================
class RandomLoop
{
public:
    RandomLoop() {}
    
    RandomLoop(int bufferLen, int countLen)
        :bufferLength(bufferLen), countLength(countLen){}
    
    void init() { counter = 0; }
    
    std::vector<int> advanceCtrAndReturn()
    {
        if (counter == 0)
        {
            for (int i = 0; i < 2; ++i)
                loopValues.push_back((rand() % 128) * (bufferLength / 128));
            
            std::for_each(loopValues.begin(),
                          loopValues.end(),
                          [this](int& value) { value %= bufferLength; });
            
            std::sort(loopValues.begin(), loopValues.end());
            
            int loopLength = loopValues.at(1) - loopValues.at(0);
            
            loopValues.at(1) = loopLength > 2 ? loopLength : 2;
        }
        
        ++counter;
        counter %= countLength;
        
        return loopValues;
    }
    
private:
    std::vector<int> loopValues { 0, 4410 };
    int bufferLength = 44100;
    int countLength = 4410;
    int counter = 0;
};

//==============================================================================
class CDSkip
{
public:
    CDSkip(int bufferLen, int bufferDiv)
        :bufferLength(bufferLen), bufferDivisions(bufferDiv)
    {
        bufferSegmentLength = bufferLength / bufferDivisions;
    }
    
    void init()
    {
        counter = 0;
        segmentCounter = 0; //?
    }
    
    std::vector<int> advanceCtrAndReturn()
    {
        if (counter == 0)
        {
            // new number of skips
            countLength = rand() % 3 + 2;
            
            // increment to next segment
            ++segmentCounter;
            segmentCounter %= (bufferDivisions - 1);
            
            loopValues.at(0) = segmentCounter * bufferSegmentLength;
            loopValues.at(1) = bufferSegmentLength;
        }
        
        // count number of skips
        ++counter;
        counter %= countLength;
        
        // limit skip end value
        std::for_each(loopValues.begin(),
                      loopValues.end(),
                      [this](int& value) { value = (value >= bufferLength ? bufferLength - 1 : value); });
        
        return loopValues;
    }
    
private:
    std::vector<int> loopValues { 0, 4410 };
    int bufferLength = 44100;
    int segmentCounter = 0;
    int bufferDivisions = 8;
    int bufferSegmentLength = 4410;
    int countLength = 3;
    int counter = 0;
    
};

//==============================================================================
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
    void setClockSpeed(float newClockSpeed);
    void setAnalogFX(float newAnalogFX);
    void setDigitalFX(float newDigitalFX);
    void setLofiFX(float newLofiFX);
    
private:
    int mBentBufferLength = 66150;
    CircularBuffer<float> mCircularBuffer { mBentBufferLength };
    
    std::vector<float> mReadPosition { 0, 0 };
    std::vector<float> mPlaybackRate = { 1.0, 1.0 };
    
    OscillatorParameters lfoParameters;
    SignalGenData<float> lfoOutput;
    
    // tape FX
    std::vector<Line<float>> tapeSpeedLine { Line<float>(), Line<float>()};
    std::vector<Line<float>> tapeStopLine { Line<float>(), Line<float>()};
    float rampTime = 6615;
    
    std::vector<float> tapeBendVals { 1.0, 0.67, 1.5, 0.5, 2.0 };
    float tapeDirMultiplier = 1;
    int clockCounter = 0;
    int clockCycle = 33075;
    
    // digital FX
    
    std::vector<RandomLoop> randomLooper { RandomLoop(mBentBufferLength, 3969), RandomLoop(mBentBufferLength, 5292) };
    std::vector<CDSkip> cdSkipper { CDSkip(mBentBufferLength, 8), CDSkip(mBentBufferLength, 8) };
    std::vector<float> skipProb = { 0, 0 };
    std::vector<int> cdSkipPlayCounter = { 0, 0 };
    std::vector<int> cdSkipPlayLength = { 4410, 4410 };
    std::vector<int> prevRandomLoop = { 0, 4410 };
    std::vector<int> randomLooperPlayCounter = { 0, 0 };
    std::vector<int> randomLooperPlayLength = { 4410, 4410 };
    
    // parameters
    float clockPeriod = 675;
    float tapeBendProb = 1;
    int tapeBendDepth = 2;
    float tapeRevProb = 0.33;
    float tapeStopProb = 0.25;
    
    float cdSkipProb = 0.15;
    float randomLoopProb = 0.35;
};

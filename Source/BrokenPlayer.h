/*
  ==============================================================================

    Broken player classes' interfaces

  ==============================================================================
*/

#pragma once

//#include <random>
#include <JuceHeader.h>
#include "CircularBuffer.h"
#include "LofiProcessors.h"
#include "Modulators.h"
#include "Utilities.h"

struct DistortionFactory
{
    std::unique_ptr<LofiProcessorBase> create(int type)
    {
        auto iter = processorMapping.find(type);
        if (iter != processorMapping.end())
            return iter->second();
        
        return nullptr;
    }
    
    std::map<int,
             std::function<std::unique_ptr<LofiProcessorBase>()>> processorMapping
    {
        { 0, []() { return std::make_unique<Bitcrusher>(); } },
        { 1, []() { return std::make_unique<SaturationProcessor>(); } }/*,
        { 2, []() { return std::make_unique<ChebyDrive>(); } }*/
    };
};

//==============================================================================
class RandomLoop
{
public:
    RandomLoop();
    
    RandomLoop(int bufferLen, int countLen);
    
    void init();
    
    std::vector<int> advanceCtrAndReturn();
    
    void setBufferLength(int newBufferLen);
    
private:
    std::vector<int> mLoopValues { 0, 4410 };
    int mBufferLength = 44100;
    int mCountLength = 4410;
    int mCounter = 0;
};

//==============================================================================
class CDSkip
{
public:
    CDSkip(int bufferLen, int bufferDiv);
    
    void init();
    
    std::vector<int> advanceCtrAndReturn();
    
    void setBufferLength(int newBufferLen);
    
    void setBufferDivisions(int newNumDivisions);
    
private:
    std::vector<int> mLoopValues { 0, 4410 };
    int mBufferLength = 44100;
    int mBufferDivisions = 8;
    int mBufferSegmentLength = 4410;
    int mSegmentCounter = 0;
    int mCounter = 0;
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
    void receiveClockedPulse();
    //==============================================================================
    void setAnalogFX(float newAnalogFX);
    void setDigitalFX(float newDigitalFX);
    void setLofiFX(float newLofiFX);
    void setDistortionType(int newDistortion);
    void setBufferLength(int newBufferLength);
    void newNumRepeats(int newRepeatCount);
    void setClockSpeed(int newClockSpeed);
    void useExternalClock(bool shouldUseExternalClock);
    
private:
    int mBentBufferLength { 66150 }; // length of full 8s buffer to use
    CircularBuffer<float> mCircularBuffer { 353312 }; // 8 seconds + 512 samples for safety
    
    std::vector<float> mReadPosition { 0, 0 };
    std::vector<float> mPlaybackRate { 1.0, 1.0 };
    //    std::vector<float> mChirpReadPosition { 0.0, 0.0 };
    
    OscillatorParameters mLfoParameters;
    SignalGenData<float> mLfoOutput;
    
    // clock
    int mClockCounter { 0 };
    int mClockCycle { 33075 };
    bool mIsTransportPlaying { false };
    juce::AudioPlayHead* mAudioPlayHead { nullptr };
    juce::Optional<juce::AudioPlayHead::PositionInfo> mPositionInfo;
    bool mShouldUseExternalClock { false };
    
    // tape FX
    std::vector<Line<float>> mTapeSpeedLine { Line<float>(), Line<float>()};
    std::vector<Line<float>> mTapeStopLine { Line<float>(), Line<float>()};
    float mRampTime { 6615 };
    std::vector<float> mTapeBendVals { 1.0, 0.67, 1.5, 0.5, 2.0 };
    float mTapeDirMultiplier { 1 };
    
    // digital FX
    //    std::vector<CDSkip> cdSkipper { CDSkip(mBentBufferLength, 4), CDSkip(mBentBufferLength, 4) };
    //    std::vector<int> cdSkipPlayCounter = { 0, 0 };
    //    std::vector<int> cdSkipPlayLength = { 4410, 4410 };
    std::vector<RandomLoop> mRandomLooper { RandomLoop(mBentBufferLength, 3308), RandomLoop(mBentBufferLength, 4410) };
    std::vector<float> mSkipProb { 0, 0 };
    CDSkip mRepeater { mBentBufferLength, 8 };
    std::vector<int> mRepeatsValues { 0, 4410 };
    std::vector<int> mRepeatsCounter = { 0, 0 };
    std::vector<int> mRepeatsPlaybackCounter { 0, 0 };
    int mNumRepeats { 1 };
    std::vector<int> mPrevRandomLoop { 0, 4410 };
    std::vector<int> mRandomLooperPlayCounter { 0, 0 };
    std::vector<int> mRandomLooperPlayLength { 4410, 4410 };
    
    // parameters
    float mClockPeriod { 675 };
    float mTapeBendProb { 1 };
    int mTapeBendDepth { 2 };
    float mTapeRevProb { 0.33 };
    float mTapeStopProb { 0.25 };
    float mRandomLoopProb { 0.35 };
    float mDistortionProb { 0 };
//    float cdSkipProb { 0.15 };
    
    // distortion processor values
    bool mUseDist { false };
    int mCurrentDist { 0 };
    int mPrevDist { -1 };
    LofiProcessorParameters mDistortionParameters;
    
    // distortion processors
    DistortionFactory mDistortionFactory {};
    std::unique_ptr<LofiProcessorBase> mSlotProcessor = std::unique_ptr<LofiProcessorBase> {};
};

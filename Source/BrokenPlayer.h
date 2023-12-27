/*
  ==============================================================================

    

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
    RandomLoop() {}
    
    RandomLoop(int bufferLen, int countLen)
        :bufferLength(bufferLen), countLength(countLen){}
    
    void init() { counter = 0; }
    
    std::vector<int> advanceCtrAndReturn()
    {
        if (counter == 0)
        {
            for (int i = 0; i < 2; ++i)
                loopValues.at(i) = (randomFloat() * 64) * ((bufferLength - 1) / 64);
            
            // normal distribution to 2 std dev - was supposed to make more
            // values close together but actually seemed to make glitches sparser
            /*
            std::random_device rd {};
            std::mt19937 gen { rd() }; // mersenne "twister" random generator
            
            for (int i = 0; i < 2; ++i)
            {
                std::normal_distribution<float> dist { static_cast<float>((bufferLength - 1) / 2), static_cast<float>((bufferLength - 1) / 4) };
                
                int randomVal = static_cast<int>(std::round(dist(gen)));
                
                loopValues.at(i) = std::clamp<int>(randomVal, 0, (bufferLength - 1), std::less<int>());
            }
            */
            std::sort(loopValues.begin(), loopValues.end(), std::less<int>());
        }
        
        ++counter;
        counter %= countLength;
        
        return loopValues;
    }
    
    void setBufferLength(int newBufferLen)
    {
        bufferLength = std::clamp<int>(newBufferLen, 0, 352800, std::less<int>());
        counter = 0;
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
            //countLength = rand() % 3 + 2;
            
            // increment to next segment
            ++segmentCounter;
            segmentCounter %= (bufferDivisions - 1);
            
            int newSegmentStart = segmentCounter * bufferSegmentLength;
            
            loopValues.at(0) = newSegmentStart >= 0 && newSegmentStart < bufferLength ? newSegmentStart : 0;
            loopValues.at(1) = bufferSegmentLength >= 4 ? bufferSegmentLength : 4;
        }
        
        // count number of skips
        ++counter;
        counter %= bufferDivisions;
        
        // limit skip end value
        std::for_each(loopValues.begin(),
                      loopValues.end(),
                      [this](int& value) { value = (value >= bufferLength ? bufferLength - 1 : value); });
        
        return loopValues;
    }
    
    void setBufferLength(int newBufferLen)
    {
        bufferLength = std::clamp<int>(newBufferLen, 0, 352800, std::less<int>());
        counter = 0;
        segmentCounter = 0;
    }
    
    void setBufferDivisions(int newNumDivisions)
    {
        bufferDivisions = newNumDivisions;
        bufferSegmentLength = bufferLength / bufferDivisions;
    }
    
private:
    std::vector<int> loopValues { 0, 4410 };
    int bufferLength = 44100;
    int bufferDivisions = 8;
    int bufferSegmentLength = 4410;
    int segmentCounter = 0;
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
    int mBentBufferLength = 66150; // length of full 8s buffer to use
    CircularBuffer<float> mCircularBuffer { 353312 }; // 8 seconds + 512 samples for safety
    
    std::vector<float> mReadPosition { 0, 0 };
    std::vector<float> mPlaybackRate { 1.0, 1.0 };
//    std::vector<float> mChirpReadPosition { 0.0, 0.0 };
    
    OscillatorParameters lfoParameters;
    SignalGenData<float> lfoOutput;
    
    // clock
    int clockCounter = 0;
    int clockCycle = 33075;
    bool isTransportPlaying = false;
    juce::AudioPlayHead* audioPlayHead = nullptr;
    juce::Optional<juce::AudioPlayHead::PositionInfo> positionInfo;
    //double bpm;
    bool shouldUseExternalClock = false;
    
    // tape FX
    std::vector<Line<float>> tapeSpeedLine { Line<float>(), Line<float>()};
    std::vector<Line<float>> tapeStopLine { Line<float>(), Line<float>()};
    float rampTime = 6615;
    std::vector<float> tapeBendVals { 1.0, 0.67, 1.5, 0.5, 2.0 };
    float tapeDirMultiplier = 1;
    
    // digital FX
    std::vector<RandomLoop> randomLooper { RandomLoop(mBentBufferLength, 3308), RandomLoop(mBentBufferLength, 4410) };
    //std::vector<CDSkip> cdSkipper { CDSkip(mBentBufferLength, 4), CDSkip(mBentBufferLength, 4) };
    std::vector<float> skipProb = { 0, 0 };
    //std::vector<int> cdSkipPlayCounter = { 0, 0 };
    //std::vector<int> cdSkipPlayLength = { 4410, 4410 };
    CDSkip repeater { mBentBufferLength, 8 };
    std::vector<int> repeatsValues { 0, 4410 };
    std::vector<int> repeatsCounter = { 0, 0 };
    std::vector<int> repeatsPlaybackCounter { 0, 0 };
    int mNumRepeats = 1;
    std::vector<int> prevRandomLoop = { 0, 4410 };
    std::vector<int> randomLooperPlayCounter = { 0, 0 };
    std::vector<int> randomLooperPlayLength = { 4410, 4410 };
    
    // parameters
    float clockPeriod = 675;
    float tapeBendProb = 1;
    int tapeBendDepth = 2;
    float tapeRevProb = 0.33;
    float tapeStopProb = 0.25;
    
//    float cdSkipProb = 0.15;
    float randomLoopProb = 0.35;
    
    float distortionProb = 0;
    
    // lofi processors
//    Bitcrusher bitcrusher;
//    LofiProcessorParameters bitcrusherParameters;
    
    bool useDist { false };
    int currentDist { 0 };
    int prevDist { -1 };
    
    DistortionFactory distortionFactory {};
    
    std::unique_ptr<LofiProcessorBase> slotProcessor = std::unique_ptr<LofiProcessorBase> {};
    
    LofiProcessorParameters distortionParameters;
};

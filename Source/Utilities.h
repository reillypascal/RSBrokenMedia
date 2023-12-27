/*
  ==============================================================================

    Miscellaneous functions and classes

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

inline float randomFloat()
{
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

inline float scale(float input, float inLow, float inHi, float outLow, float outHi)
{
    float scaleFactor = (outHi - outLow)/(inHi - inLow);
    float offset = outLow - inLow;
    return (input * scaleFactor) + offset;
}

inline float wrap(float a, float b)
{
    float mod = fmodf(a, b);
    return (a >= 0 ? 0 : b) + (mod > __FLT_EPSILON__ || !isnan(mod) ? mod : 0);
}

struct LofiProcessorParameters
{
    LofiProcessorParameters() {}
    
    LofiProcessorParameters& operator=(const LofiProcessorParameters& params)
    {
        if (this != &params)
        {
            bitDepth = params.bitDepth;
//            bitrate = params.bitrate;
            downsampling = params.downsampling;
            drive = params.drive;
//            waveshape = params.waveshape;
        }
        return *this;
    }
    int bitDepth { 24 };
//    int bitrate { 1 };
    int downsampling { 1 };
    float drive { 0.0f };
//    int waveshape { 2 };
};

class LofiProcessorBase
{
public:
    LofiProcessorBase() {}
    
    virtual ~LofiProcessorBase() {}
    
    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;
    
    virtual void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) = 0;
    
    virtual void reset() = 0;
    
    virtual LofiProcessorParameters& getParameters() = 0;
    
    virtual void setParameters(const LofiProcessorParameters& params) = 0;
};

class LockGuardedPosInfo
{
public:
    void set(const juce::AudioPlayHead::PositionInfo& newInfo)
    {
        const std::lock_guard<std::mutex> lock(mMutex);
        mInfo = newInfo;
    }
    
    juce::AudioPlayHead::PositionInfo get() noexcept
    {
        const std::lock_guard<std::mutex> lock(mMutex);
        return mInfo;
    }
private:
    std::mutex mMutex;
    juce::AudioPlayHead::PositionInfo mInfo;
};

/*
inline float randomFloatStdDev(float mean = 0, float stdDev = 1)
{
    std::vector<float> randomNos;
    for (int i = 0; i < 2; ++i)
        randomNos.at(i) = randomFloat();
    
    float randWithStdDev = (pow(-2 * std::log(randomNos.at(0)), 0.5) * std::cos(2 * M_PI * randomNos.at(1)));
    return (randWithStdDev * stdDev) + mean;
}
*/

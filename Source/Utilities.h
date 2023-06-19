/*
  ==============================================================================

    Utilities.h
    Created: 12 Jun 2023 1:29:35am
    Author:  Reilly Spitzfaden

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// measures change between sequential values
// functor so it can keep state
/*
template <typename T>
struct Delta
{
    T prevVal;
    
    Delta() : prevVal(0) {}
    
    T operator()(const T& currentVal)
    {
        T delta = currentVal - prevVal;
        if (delta <= __FLT_EPSILON__ || isnan(delta))
            delta = 0;
        prevVal = currentVal;
        
        return delta;
    }
};
*/
inline float randomFloat()
{
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}
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

class SpinLockedPosInfo
{
public:
    // Wait-free, but setting new info may fail if the main thread is currently
    // calling `get`. This is unlikely to matter in practice because
    // we'll be calling `set` much more frequently than `get`.
    void set (const juce::AudioPlayHead::PositionInfo& newInfo)
    {
        const juce::SpinLock::ScopedTryLockType lock (mutex);

        if (lock.isLocked())
            info = newInfo;
    }

    juce::AudioPlayHead::PositionInfo get() const noexcept
    {
        const juce::SpinLock::ScopedLockType lock (mutex);
        return info;
    }

private:
    juce::SpinLock mutex;
    juce::AudioPlayHead::PositionInfo info;
};

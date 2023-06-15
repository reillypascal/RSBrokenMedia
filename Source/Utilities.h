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

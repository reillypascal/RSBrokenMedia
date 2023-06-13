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

inline float scale(float input, float inLow, float inHi, float outLow, float outHi)
{
    float scaleFactor = (outHi - outLow)/(inHi - inLow);
    float offset = outLow - inLow;
    return (input * scaleFactor) + offset;
}

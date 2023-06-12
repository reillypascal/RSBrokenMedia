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
template <typename T>
struct Delta
{
    T prevVal;
    
    Delta() : prevVal(0) {}
    
    T operator()(const T& currentVal)
    {
        T delta = currentVal - prevVal;
        prevVal = currentVal;
        
        return delta;
    }
};

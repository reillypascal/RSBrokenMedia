/*
  ==============================================================================

 Ramping random modulation source

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// pure virtual base class
template <typename T>
class IAudioSignalGenerator
{
public:
    virtual ~IAudioSignalGenerator() = default;
    
    virtual bool reset(double _sampleRate) = 0;
    virtual const T renderAudioOutput() = 0;
};

//==============================================================================
struct RampParameters
{
    RampParameters() {}
    
    RampParameters& operator=(const RampParameters& params)
    {
        if (this == &params)
            return *this;
        
        rampTime = params.rampTime;
        clock_Hz = params.clock_Hz;
        return *this;
    }
    
    float rampTime = 150;
    float clock_Hz = 1.5;
};

//==============================================================================
template <typename SampleType>
class RampRand : public IAudioSignalGenerator<SampleType>
{
public:
    RampRand();
    virtual ~RampRand();
    
    virtual bool reset(SampleType _sampleRate);
    
    RampParameters getParameters();
    
    void setParameters(const RampParameters& params);
    
    virtual const SampleType renderAudioOutput();
    
private:
    RampParameters rampParameters;
    
    SampleType sampleRate = 44100;
    
    SampleType modCounter = 0.0;
    SampleType phaseInc = 0.0;
    
    inline bool checkAndWrapModulo(SampleType& moduloCounter, SampleType phaseInc);
    
    inline bool advanceAndCheckWrapModulo(SampleType& moduloCounter, SampleType phaseInc);
    
    inline void advanceModulo(SampleType& moduloCounter, SampleType phaseInc);
};

/*
  ==============================================================================

 Ramping random modulation source interface

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// pure virtual base class
/*
template <typename SampleType>
class IAudioSignalGenerator
{
public:
    virtual ~IAudioSignalGenerator() = default;
    
    virtual bool reset(double _sampleRate) = 0;
    virtual const SampleType renderAudioOutput() = 0;
};
*/

// LFO output data
//==============================================================================
template <typename SampleType>
struct SignalGenData
{
    SignalGenData() {}
    
    SampleType normalOutput = 0.0;
    SampleType invertedOutput = 0.0;
    SampleType quadPhaseOutput_pos = 0.0;
    SampleType quadPhaseOutput_neg = 0.0;
};

// LFO parameters
//==============================================================================
enum class generatorWaveform { kTriangle, kSin, kSaw };

struct OscillatorParameters
{
    OscillatorParameters() {}
    
    OscillatorParameters& operator=(const OscillatorParameters& params)
    {
        if (this == &params)
            return *this;
        
        waveform = params.waveform;
        frequency_Hz = params.frequency_Hz;
        return *this;
    }
    
    generatorWaveform waveform = generatorWaveform::kTriangle;
    double frequency_Hz = 0.0;
};

// LFO mapping functions
//==============================================================================
template <typename SampleType>
inline SampleType unipolarToBipolar(SampleType value)
{
    return 2.0 * value - 1.0;
}

template <typename SampleType>
inline SampleType bipolarToUnipolar(SampleType value)
{
    return 0.5 * value + 0.5;
}

//================ LFO ================
//==============================================================================
template <typename SampleType>
class LFO// : public IAudioSignalGenerator<SampleType>
{
public:
    LFO();
    virtual ~LFO();
    
    virtual bool reset(SampleType _sampleRate);
    
    OscillatorParameters getParameters();
    
    void setParameters(const OscillatorParameters& params);
    
    const SignalGenData<SampleType> renderAudioOutput();
    
protected:
    SampleType mSampleRate = 0.0;
    
    OscillatorParameters mLfoParameters;
    
    SampleType mModCounter = 0.0;
    SampleType mPhaseInc = 0.0;
    SampleType mModCounterQP = 0.0;
    
    inline bool checkAndWrapModulo(SampleType& moduloCounter, SampleType phaseInc);
    
    inline bool advanceAndCheckWrapModulo(SampleType& moduloCounter, SampleType phaseInc);
    
    inline void advanceModulo(SampleType& moduloCounter, SampleType phaseInc);
    
    const SampleType mB = 4.0 / M_PI;
    const SampleType mC = -4.0 / (M_PI / M_PI);
    const SampleType mP = 0.225;
    
    inline SampleType parabolicSine(SampleType angle);
};

//================ Line with start/end and ramp time ================
//==============================================================================
template <typename SampleType>
class Line// : public IAudioSignalGenerator<SampleType>
{
public:
    Line();
    virtual ~Line();
    
    virtual bool reset(SampleType _sampleRate);
    
    SampleType getParameters();
    
    void setParameters(const SampleType& newRampTime);
    
    void setDestination(const SampleType& newDestination);
    
    virtual const SampleType renderAudioOutput();
        
private:
    int mSampleRate = 44100;
    
    SampleType mPhaseInc = 0.0;
    SampleType mRampTimeSecs = 0.15;
    SampleType mRampTimeSamps = 6615;
    
    SampleType mStartingValue = 1.0;
    SampleType mDestinationValue = 1.0;
    
    SampleType mOutput = 1.0;
};

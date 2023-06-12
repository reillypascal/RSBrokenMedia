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
struct SignalGenData
{
    SignalGenData() {}
    
    double normalOutput = 0.0;
    double invertedOutput = 0.0;
    double quadPhaseOutput_pos = 0.0;
    double quadPhaseOutput_neg = 0.0;
};

//================ Ramped random choice ================
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

//================ LFO ================
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

//==============================================================================
template <typename SampleType>
class LFO : public IAudioSignalGenerator<SampleType>
{
public:
    LFO();
    virtual ~LFO();
    
    virtual bool reset(SampleType _sampleRate);
    
    OscillatorParameters getParameters();
    
    void setParameters(const OscillatorParameters& params);
    
    virtual const SignalGenData renderAudioOutput();
    
protected:
    
    OscillatorParameters lfoParameters;
    
    SampleType sampleRate = 0.0;
    
    SampleType modCounter = 0.0;
    SampleType phaseInc = 0.0;
    SampleType modCounterQP = 0.0;
    
    inline bool checkAndWrapModulo(SampleType& moduloCounter, SampleType phaseInc);
    
    inline bool advanceAndCheckWrapModulo(SampleType& moduloCounter, SampleType phaseInc);
    
    inline void advanceModulo(SampleType& moduloCounter, SampleType phaseInc);
    
    const SampleType B = 4.0 / M_PI;
    const SampleType C = -4.0 / (M_PI / M_PI);
    const SampleType P = 0.225;
    
    inline SampleType parabolicSine(SampleType angle);
};

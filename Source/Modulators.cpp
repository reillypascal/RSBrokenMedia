/*
  ==============================================================================

    

  ==============================================================================
*/

#include "Modulators.h"
#include "Utilities.h"

//================ LFO ================
//==============================================================================
template <typename SampleType>
LFO<SampleType>::LFO() { srand(static_cast<uint32_t>(time(NULL))); }

template <typename SampleType>
LFO<SampleType>::~LFO() = default;

template <typename SampleType>
bool LFO<SampleType>::reset(SampleType _sampleRate)
{
    sampleRate = _sampleRate;
    phaseInc = lfoParameters.frequency_Hz / sampleRate;
    
    modCounter = 0.0;
    modCounterQP = 0.25;
    
    return true;
}

template <typename SampleType>
OscillatorParameters LFO<SampleType>::getParameters() { return lfoParameters; }

template <typename SampleType>
void LFO<SampleType>::setParameters(const OscillatorParameters& params)
{
    if (params.frequency_Hz != lfoParameters.frequency_Hz)
        phaseInc = params.frequency_Hz / sampleRate;
    
    lfoParameters = params;
}

template <typename SampleType>
const SignalGenData LFO<SampleType>::renderAudioOutput()
{
    checkAndWrapModulo(modCounter, phaseInc);
    
    modCounterQP = modCounter;
    
    advanceAndCheckWrapModulo(modCounterQP, 0.25);
    
    SignalGenData output;
    
    if (lfoParameters.waveform == generatorWaveform::kSin)
    {
        SampleType angle = modCounter * 2.0 * M_PI - M_PI;
        
        output.normalOutput = parabolicSine(-angle);
        
        angle = modCounterQP * 2.0 * M_PI - M_PI;
        
        output.quadPhaseOutput_pos = parabolicSine(-angle);
    }
    else if (lfoParameters.waveform == generatorWaveform::kTriangle)
    {
        // bipolar saw
        output.normalOutput = unipolarToBipolar<SampleType>(modCounter);
        // bipolar triangle from saw
        output.normalOutput = 2.0 * fabs(output.normalOutput) - 1.0;
        
        output.quadPhaseOutput_pos = unipolarToBipolar<SampleType>(modCounterQP);
        
        output.quadPhaseOutput_pos = 2.0 * fabs(output.quadPhaseOutput_pos) -1.0;
    }
    else if (lfoParameters.waveform == generatorWaveform::kSaw)
    {
        output.normalOutput = unipolarToBipolar<SampleType>(modCounter);
        
        output.quadPhaseOutput_pos = unipolarToBipolar<SampleType>(modCounterQP);
    }
    
    output.quadPhaseOutput_neg = -output.quadPhaseOutput_pos;
    output.invertedOutput = -output.normalOutput;
    
    advanceModulo(modCounter, phaseInc);
    
    return output;
}

template <typename SampleType>
inline bool LFO<SampleType>::checkAndWrapModulo(SampleType& moduloCounter, SampleType phaseInc)
{
    if (phaseInc > 0 && moduloCounter >= 1.0)
    {
        moduloCounter -= 1.0;
        return true;
    }
    
    if (phaseInc < 0 && moduloCounter <= 0.0)
    {
        moduloCounter += 1.0;
        return true;
    }
    
    return false;
}

template <typename SampleType>
inline bool LFO<SampleType>::advanceAndCheckWrapModulo(SampleType& moduloCounter, SampleType phaseInc)
{
    moduloCounter += phaseInc;
    
    if (phaseInc > 0 && moduloCounter >= 1.0)
    {
        moduloCounter -= 1.0;
        return true;
    }
    
    if (phaseInc < 0 && moduloCounter >= 1.0)
    {
        moduloCounter += 1.0;
        return true;
    }
    
    return false;
}

template <typename SampleType>
inline void LFO<SampleType>::advanceModulo(SampleType& moduloCounter, SampleType phaseInc) { moduloCounter += phaseInc; }

template <typename SampleType>
inline SampleType LFO<SampleType>::parabolicSine(SampleType angle)
{
    SampleType y = B * angle + C * angle * fabs(angle);
    y = P * (y * fabs(y) - y) + y;
    return y;
}

//================ Line with start/end and ramp time ================
//==============================================================================
template <typename SampleType>
Line<SampleType>::Line() {}

template <typename SampleType>
Line<SampleType>::~Line() = default;

template <typename SampleType>
bool Line<SampleType>::reset(SampleType _sampleRate)
{
    sampleRate = _sampleRate;
    phaseInc = (1000 / rampTime) / sampleRate;
    
    return true;
}

template <typename SampleType>
SampleType Line<SampleType>::getParameters() { return rampTime; }

template <typename SampleType>
void Line<SampleType>::setParameters(const SampleType& newRampTime)
{
    if (rampTime != newRampTime)
        rampTime = newRampTime;
}

template <typename SampleType>
void Line<SampleType>::setDestination(const SampleType& newDestination)
{
    if (destinationValue != newDestination)
    {
        destinationValue = newDestination;
        startingValue = output;
    }
    
    phaseInc = ((1000 / rampTime) / sampleRate) * (destinationValue - output);
}

template <typename SampleType>
const SampleType Line<SampleType>::renderAudioOutput()
{
    if (startingValue < destinationValue)
    {
        if (output < destinationValue)
            output += phaseInc;
        else
            output = destinationValue;
    }
    else
    {
        if (output > destinationValue)
            output += phaseInc;
        else
            output = destinationValue;
    }
}

/*
  ==============================================================================

 Ramping random modulation source implementation

  ==============================================================================
*/

#include "Modulators.h"

//================ LFO ================
//==============================================================================
template <typename SampleType>
LFO<SampleType>::LFO() { srand(static_cast<uint32_t>(time(NULL))); }

template <typename SampleType>
LFO<SampleType>::~LFO() = default;

template <typename SampleType>
bool LFO<SampleType>::reset(SampleType _sampleRate)
{
    mSampleRate = _sampleRate;
    mPhaseInc = mLfoParameters.frequency_Hz / mSampleRate;
    
    mModCounter = 0.0;
    mModCounterQP = 0.25;
    
    return true;
}

template <typename SampleType>
OscillatorParameters LFO<SampleType>::getParameters() { return mLfoParameters; }

template <typename SampleType>
void LFO<SampleType>::setParameters(const OscillatorParameters& params)
{
    if (params.frequency_Hz != mLfoParameters.frequency_Hz)
        mPhaseInc = params.frequency_Hz / mSampleRate;
    
    mLfoParameters = params;
}

template <typename SampleType>
const SignalGenData<SampleType> LFO<SampleType>::renderAudioOutput()
{
    checkAndWrapModulo(mModCounter, mPhaseInc);
    
    mModCounterQP = mModCounter;
    
    advanceAndCheckWrapModulo(mModCounterQP, 0.25);
    
    SignalGenData<SampleType> output;
    
    if (mLfoParameters.waveform == generatorWaveform::kSin)
    {
        SampleType angle = mModCounter * 2.0 * M_PI - M_PI;
        
        output.normalOutput = parabolicSine(-angle);
        
        angle = mModCounterQP * 2.0 * M_PI - M_PI;
        
        output.quadPhaseOutput_pos = parabolicSine(-angle);
    }
    else if (mLfoParameters.waveform == generatorWaveform::kTriangle)
    {
        // bipolar saw
        output.normalOutput = unipolarToBipolar<SampleType>(mModCounter);
        // bipolar triangle from saw
        output.normalOutput = 2.0 * fabs(output.normalOutput) - 1.0;
        
        output.quadPhaseOutput_pos = unipolarToBipolar<SampleType>(mModCounterQP);
        
        output.quadPhaseOutput_pos = 2.0 * fabs(output.quadPhaseOutput_pos) -1.0;
    }
    else if (mLfoParameters.waveform == generatorWaveform::kSaw)
    {
        output.normalOutput = unipolarToBipolar<SampleType>(mModCounter);
        
        output.quadPhaseOutput_pos = unipolarToBipolar<SampleType>(mModCounterQP);
    }
    
    output.quadPhaseOutput_neg = -output.quadPhaseOutput_pos;
    output.invertedOutput = -output.normalOutput;
    
    advanceModulo(mModCounter, mPhaseInc);
    
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
    SampleType y = mB * angle + mC * angle * fabs(angle);
    y = mP * (y * fabs(y) - y) + y;
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
    mSampleRate = _sampleRate;
    //rampTimeSamps = sampleRate * rampTimeSecs;
    
    return true;
}

template <typename SampleType>
SampleType Line<SampleType>::getParameters() { return mRampTimeSamps; }

template <typename SampleType>
void Line<SampleType>::setParameters(const SampleType& newRampTime)
{
    if (mRampTimeSamps != newRampTime)
        mRampTimeSamps = newRampTime;
    
    //rampTimeSamps = sampleRate * rampTimeSecs;
}

template <typename SampleType>
void Line<SampleType>::setDestination(const SampleType& newDestination)
{
    if (mDestinationValue != newDestination)
    {
        mDestinationValue = newDestination;
        mStartingValue = mOutput;
    }
    
    mPhaseInc = (mDestinationValue - mStartingValue) / mRampTimeSamps;
}

template <typename SampleType>
const SampleType Line<SampleType>::renderAudioOutput()
{
    if (mStartingValue < mDestinationValue)
    {
        if (mOutput < mDestinationValue)
            mOutput += mPhaseInc;
        else
            mOutput = mDestinationValue;
    }
    else
    {
        if (mOutput > mDestinationValue)
            mOutput += mPhaseInc;
        else
            mOutput = mDestinationValue;
    }
    
    return mOutput;
}

template class LFO<double>;
template class LFO<float>;

template class Line<double>;
template class Line<float>;

/*
  ==============================================================================
 
 Circular buffer implementation
 
  ==============================================================================
*/

#include "CircularBuffer.h"

template <typename SampleType>
CircularBuffer<SampleType>::CircularBuffer(int bufferSize)
{
    jassert(bufferSize >= 0);
    
    mTotalSize = bufferSize > 4 ? bufferSize : 4;
    mCircularBuffer.setSize(static_cast<int>(mCircularBuffer.getNumChannels()), mTotalSize, false, false, false);
    mCircularBuffer.clear();
    mNumSamples = mCircularBuffer.getNumSamples();
}

//==============================================================================
template <typename SampleType>
void CircularBuffer<SampleType>::prepare (const juce::dsp::ProcessSpec& spec)
{
    jassert(spec.numChannels > 0);
    
    mCircularBuffer.setSize(static_cast<int>(spec.numChannels), mTotalSize, false, false, false);
    
    mWritePosition.resize(spec.numChannels);
    
    mSampleRate = spec.sampleRate;
    
    reset();
}

//==============================================================================
template <typename SampleType>
void CircularBuffer<SampleType>::reset()
{
    std::fill(mWritePosition.begin(), mWritePosition.end(), 0);
    
    mCircularBuffer.clear();
}

//==============================================================================
template <typename SampleType>
void CircularBuffer<SampleType>::fillNextBlock(int channel, const int inBufferLength, const SampleType* inBufferData)
{
    if (mUsedSegmentLength > inBufferLength + mWritePosition.at(channel))
    {
        mCircularBuffer.copyFromWithRamp(channel, mWritePosition.at(channel), inBufferData, inBufferLength, 1, 1);
    }
    else
    {
        const int bufferRemaining = mUsedSegmentLength - mWritePosition.at(channel);
        
        mCircularBuffer.copyFromWithRamp(channel, mWritePosition.at(channel), inBufferData, bufferRemaining, 1, 1);
        mCircularBuffer.copyFromWithRamp(channel, 0, inBufferData, inBufferLength - bufferRemaining, 1, 1);
    }
    
    mWritePosition.at(channel) += inBufferLength;
    mWritePosition.at(channel) %= mUsedSegmentLength;
}

//==============================================================================
template <typename SampleType>
const SampleType CircularBuffer<SampleType>::readSample(int channel, SampleType readPosition)
{
    // look at DelayLine implementation
    mReadPosFrac = readPosition - floor(readPosition);
    mReadPosInt = static_cast<int>(floor(readPosition));
    
    int index1 = mReadPosInt;
    int index2 = mReadPosInt + 1;
    
    if (index2 >= mUsedSegmentLength)
    {
        index1 %= mUsedSegmentLength;
        index2 %= mUsedSegmentLength;
    }
            
    SampleType value1 = mCircularBuffer.getSample(channel, index1);
    SampleType value2 = mCircularBuffer.getSample(channel, index2);
    
    // add difference between samples scaled by position between them
    return value1 + (mReadPosFrac * (value2 - value1));
}

//==============================================================================
template <typename SampleType>
const int CircularBuffer<SampleType>::getBufferSize()
{
    return mCircularBuffer.getNumSamples();
}

//==============================================================================
template <typename SampleType>
const juce::String CircularBuffer<SampleType>::getName() const { return "CircularBuffer"; };

//==============================================================================
template <typename SampleType>
void CircularBuffer<SampleType>::setUsedBufferSegmentLength(const int newSegmentLength)
{
    mUsedSegmentLength = std::clamp<int>(newSegmentLength, 0, mTotalSize - 512, std::less<int>());
}

template class CircularBuffer<float>;
template class CircularBuffer<double>;

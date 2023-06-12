/*
  ==============================================================================

 - need gain (from copyFromWithRamp)?
 - split into cpp/h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

template <typename SampleType>
class CircularBuffer
{
public:
    CircularBuffer(int bufferSize)
    {
        jassert(bufferSize >= 0);
        
        mTotalSize = bufferSize > 4 ? bufferSize : 4;
        mCircularBuffer.setSize(static_cast<int>(mCircularBuffer.getNumChannels()), mTotalSize, false, false, false);
        mCircularBuffer.clear();
        mNumSamples = mCircularBuffer.getNumSamples();
    }
    
    ~CircularBuffer() = default;
    
    //==============================================================================
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        jassert(spec.numChannels > 0);
        
        mCircularBuffer.setSize(static_cast<int>(spec.numChannels), mTotalSize, false, false, false);
        
        mWritePosition.resize(spec.numChannels);
        
        mSampleRate = spec.sampleRate;
        
        reset();
    }
    
    //==============================================================================
    void reset()
    {
        std::fill(mWritePosition.begin(), mWritePosition.end(), 0);
        
        mCircularBuffer.clear();
    }
    
    //==============================================================================
    void fillNextBlock(int channel, const int inBufferLength, const SampleType* inBufferData)
    {
        if (mCircularBuffer.getNumSamples() > inBufferLength + mWritePosition.at(channel))
        {
            mCircularBuffer.copyFromWithRamp(channel, mWritePosition.at(channel), inBufferData, inBufferLength, 1, 1);
        }
        else
        {
            const int bufferRemaining = mCircularBuffer.getNumSamples() - mWritePosition.at(channel);
            
            mCircularBuffer.copyFromWithRamp(channel, mWritePosition.at(channel), inBufferData, bufferRemaining, 1, 1);
            mCircularBuffer.copyFromWithRamp(channel, 0, inBufferData, inBufferLength - bufferRemaining, 1, 1);
        }
        
        mWritePosition.at(channel) += inBufferLength;
        mWritePosition.at(channel) %= mTotalSize;
    }
    
    //==============================================================================
    SampleType readSample(int channel, SampleType readPosition)
    {
        // look at DelayLine implementation
        readPosFrac = readPosition - floor(readPosition);
        readPosInt = static_cast<int>(floor(readPosition));
        
        int index1 = readPosInt;
        int index2 = readPosInt + 1;
        
        if (index2 >= mTotalSize)
        {
            index1 %= mTotalSize;
            index2 %= mTotalSize;
        }
                
        SampleType value1 = mCircularBuffer.getSample(channel, index1);
        SampleType value2 = mCircularBuffer.getSample(channel, index2);
        
        // add difference between samples scaled by position between them
        return value1 + (readPosFrac * (value2 - value1));
    }
    
    //==============================================================================
    int getBufferSize()
    {
        return mCircularBuffer.getNumSamples();
    }
    
    //==============================================================================
    const juce::String getName() const { return "CircularBuffer"; };
    
private:
    juce::AudioBuffer<SampleType> mCircularBuffer;
    
    std::vector<int> mWritePosition { 0, 0 };
    int mSampleRate { 44100 };
    
    int mNumSamples { 0 };
    int mTotalSize { 0 };
    
    SampleType readPosFrac { 0 };
    int readPosInt { 0 };
};

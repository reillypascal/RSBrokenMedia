/*
  ==============================================================================
 
 Circular buffer interface
 - need gain (from copyFromWithRamp)?

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

template <typename SampleType>
class CircularBuffer
{
public:
    CircularBuffer(int bufferSize);
    
    ~CircularBuffer() = default;
    
    //==============================================================================
    void prepare (const juce::dsp::ProcessSpec& spec);
    
    //==============================================================================
    void reset();
    
    //==============================================================================
    void fillNextBlock(int channel, const int inBufferLength, const SampleType* inBufferData);
    
    //==============================================================================
    const SampleType readSample(int channel, SampleType readPosition);
    
    //==============================================================================
    const int getBufferSize();
    
    //==============================================================================
    const juce::String getName() const;
    
    //==============================================================================
    void setUsedBufferSegmentLength(const int newSegmentLength);
private:
    juce::AudioBuffer<SampleType> mCircularBuffer;
    
    std::vector<int> mWritePosition { 0, 0 };
    int mSampleRate { 44100 };
    
    int mNumSamples { 0 };
    int mTotalSize { 0 };
    int mUsedSegmentLength { 66150 };
    
    SampleType mReadPosFrac { 0 };
    int mReadPosInt { 0 };
};

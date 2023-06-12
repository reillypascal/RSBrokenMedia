/*
  ==============================================================================

    BrokenPlayer.cpp
    Created: 11 Jun 2023 12:08:00am
    Author:  Reilly Spitzfaden

  ==============================================================================
*/

#include "BrokenPlayer.h"

BrokenPlayer::BrokenPlayer()
{
    
}

void BrokenPlayer::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    
    mCircularBuffer.prepare(spec);
    
    mReadPosition.resize(getTotalNumInputChannels());
    mPlaybackRate.resize(getTotalNumInputChannels());
    
    std::fill(mReadPosition.begin(), mReadPosition.end(), 0);
    std::fill(mPlaybackRate.begin(), mPlaybackRate.end(), 1.0);
}

void BrokenPlayer::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        
        mCircularBuffer.fillNextBlock(channel, buffer.getNumSamples(), channelData);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = mCircularBuffer.readSample(channel, mReadPosition.at(channel));
            
            mReadPosition.at(channel) += mPlaybackRate.at(channel);
            mReadPosition.at(channel) = wrap(mReadPosition.at(channel), static_cast<float>(mGranBufferLength));
        }
    }
}

const juce::String BrokenPlayer::getName() const
{
    return "BrokenPlayer";
}

float BrokenPlayer::wrap(float a, float b)
{
    float remainder = a - (floor(a/b) * b);
    
    return (remainder < 0) ? remainder + b : remainder;
}

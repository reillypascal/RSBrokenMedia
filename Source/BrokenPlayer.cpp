/*
  ==============================================================================

TODO:
 - adjust random weighting

  ==============================================================================
*/

#include "BrokenPlayer.h"

BrokenPlayer::BrokenPlayer() {}

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
    
    lfoParameters.frequency_Hz = 1.5;
    lfoParameters.waveform = generatorWaveform::kSaw;
    lfo.setParameters(lfoParameters);
    lfo.reset(getSampleRate());
    
    leftLine.setParameters(rampTime);
    rightLine.setParameters(rampTime);
    leftLine.reset(getSampleRate());
    rightLine.reset(getSampleRate());
}

void BrokenPlayer::releaseResources() {}

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
    }
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        lfoOutput = lfo.renderAudioOutput();
        
        float delta = deltaObj(lfoOutput.normalOutput);
        if (delta < 0.0f)
        {
            // adjust random weighting
            leftLine.setDestination(tapeBendVals.at(rand() % tapeBendVals.size()));
            rightLine.setDestination(tapeBendVals.at(rand() % tapeBendVals.size()));
        }
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer (channel);
            
            channelData[sample] = mCircularBuffer.readSample(channel, mReadPosition.at(channel));
            
            if (channel == 0)
                mPlaybackRate.at(0) = leftLine.renderAudioOutput();
            else if (channel == 1)
                mPlaybackRate.at(1) = rightLine.renderAudioOutput();
            
            mReadPosition.at(channel) += mPlaybackRate.at(channel);
            mReadPosition.at(channel) = wrap(mReadPosition.at(channel), static_cast<float>(mGranBufferLength));
        }
    }
}

void BrokenPlayer::reset()
{
    lfo.reset(getSampleRate());
    
    leftLine.reset(getSampleRate());
    rightLine.reset(getSampleRate());
}

juce::AudioProcessorEditor* BrokenPlayer::createEditor() { return nullptr; }
bool BrokenPlayer::hasEditor() const { return false; }

const juce::String BrokenPlayer::getName() const { return "BrokenPlayer"; }
bool BrokenPlayer::acceptsMidi() const { return false; }
bool BrokenPlayer::producesMidi() const { return false; }
double BrokenPlayer::getTailLengthSeconds() const { return 0; }

int BrokenPlayer::getNumPrograms() { return 0; }
int BrokenPlayer::getCurrentProgram() { return 0; }
void BrokenPlayer::setCurrentProgram(int) {}
const juce::String BrokenPlayer::getProgramName(int) { return {}; }
void BrokenPlayer::changeProgramName(int, const juce::String&) {}

void BrokenPlayer::getStateInformation(juce::MemoryBlock&) {}
void BrokenPlayer::setStateInformation(const void*, int) {}

float BrokenPlayer::wrap(float a, float b)
{
    float remainder = a - (floor(a/b) * b);
    
    return (remainder < 0) ? remainder + b : remainder;
}

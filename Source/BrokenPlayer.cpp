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
    
    clockCycle = static_cast<int>(clockPeriod * 44.1);
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        
        mCircularBuffer.fillNextBlock(channel, buffer.getNumSamples(), channelData);
    }
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        if (clockCounter == 0)
        {
            // adjust random weighting
            float leftDest = tapeBendVals.at(rand() % tapeBendVals.size());
            if (rand() % 1 < tapeRevProb)
                leftDest *= -1;
            float rightDest = tapeBendVals.at(rand() % tapeBendVals.size());
            if (rand() % 1 < tapeRevProb)
                rightDest *= -1;
            
            leftLine.setDestination(leftDest);
            rightLine.setDestination(rightDest);
        }
        clockCounter++;
        clockCounter %= clockCycle;
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer (channel);
            
            if (channel == 0)
                mPlaybackRate.at(0) = leftLine.renderAudioOutput();
            else if (channel == 1)
                mPlaybackRate.at(1) = rightLine.renderAudioOutput();
            
            channelData[sample] = mCircularBuffer.readSample(channel, mReadPosition.at(channel));
            
            mReadPosition.at(channel) += mPlaybackRate.at(channel);
            mReadPosition.at(channel) = wrap(mReadPosition.at(channel), static_cast<float>(mGranBufferLength));
        }
    }
}

void BrokenPlayer::reset()
{    
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
    float mod = fmodf(a, b);
    return (a >= 0 ? 0 : b) + (mod > __FLT_EPSILON__ || !isnan(mod) ? mod : 0);
}

/*
  ==============================================================================

TODO:
 - add digital
 - add lofi
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
    
    std::for_each(tapeSpeedLine.begin(),
                  tapeSpeedLine.end(),
                  [this](auto& line)
    {
        line.setParameters.setParameters(rampTime);
        line.reset(getSampleRate());
    });
    
    std::for_each(tapeStopLine.begin(),
                  tapeStopLine.end(),
                  [this](auto& line)
    {
        line.setParameters.setParameters(rampTime);
        line.reset(getSampleRate());
    });
    
    srand(static_cast<uint32_t>(time(NULL)));
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
            // set new destinations for L/R tape speeds
            std::for_each(tapeSpeedLine.begin(),
                          tapeSpeedLine.end(),
                          [this](Line<float>& line)
            {
                if (randomFloat() < tapeBendProb)
                {
                    int index = rand() % tapeBendDepth;
                    float dest = tapeBendVals.at(index);
                    line.setDestination(dest);
                }
            });
            
            if (randomFloat() < tapeRevProb)
                tapeDirMultiplier = -1;
            else
                tapeDirMultiplier = 1;
            
            // initialize L/R tape stops
            std::for_each(tapeStopLine.begin(),
                          tapeStopLine.end(),
                          [this](Line<float>& line)
            {
                if (randomFloat() < tapeStopProb)
                {
                    line.setParameters(rampTime);
                    line.setDestination(0);
                }
            });
            
            std::for_each(skipProb.begin(),
                          skipProb.end(),
                          [this](float& prob){ prob = randomFloat(); });
        }
        clockCounter++;
        clockCounter %= clockCycle;
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer (channel);
            
            // ramp back up if stop completed
            float tapeStopSpeed = tapeStopLine.at(channel).renderAudioOutput();
            if (tapeStopSpeed < 0.01)
            {
                tapeStopLine.at(channel).setParameters(133);
                tapeStopLine.at(channel).setDestination(1.0f);
            }
            
            mPlaybackRate.at(channel) = tapeSpeedLine.at(channel).renderAudioOutput() * tapeDirMultiplier;
            mPlaybackRate.at(channel) *= tapeStopSpeed;
            
            // skips
            if (skipProb.at(channel) < cdSkipProb)
            {
                // read (and start next slice if necessary) before incrementing and wrapping
                if (cdSkipPlayCounter.at(channel) == 0)
                {
                    std::vector<int> skipLoop = cdSkipper.at(channel).advanceCtrAndReturn();
                    
                    mReadPosition.at(channel) = skipLoop.at(0);
                    cdSkipPlayLength.at(channel) = skipLoop.at(1);
                    
                    channelData[sample] = mCircularBuffer.readSample(channel, mReadPosition.at(channel));
                }
                else
                {
                    channelData[sample] = mCircularBuffer.readSample(channel, mReadPosition.at(channel));
                }
                
                // increment/wrap read position
                mReadPosition.at(channel) += mPlaybackRate.at(channel);
                mReadPosition.at(channel) = wrap(mReadPosition.at(channel), static_cast<float>(mBentBufferLength));
                // increment/wrap skip timing counter
                ++cdSkipPlayCounter.at(channel);
                cdSkipPlayCounter.at(channel) %= cdSkipPlayLength.at(channel);
            }
            // loops
            else if (skipProb.at(channel) < randomLoopProb)
            {
                std::vector<int> randomLoop = randomLooper.at(channel).advanceCtrAndReturn();
                
                if (randomLooperPlayCounter.at(channel) == 0)
                    mReadPosition.at(channel) = randomLoop.at(0);
                
                if (randomLoop != prevRandomLoop)
                {
                    prevRandomLoop = randomLoop;
                    
                    mReadPosition.at(channel) = randomLoop.at(0);
                    randomLooperPlayLength.at(channel) = randomLoop.at(1);
                    randomLooperPlayCounter.at(channel) = 0;
                }
                
                channelData[sample] = mCircularBuffer.readSample(channel, mReadPosition.at(channel));
                
                // increment/wrap read position
                mReadPosition.at(channel) += mPlaybackRate.at(channel);
                mReadPosition.at(channel) = wrap(mReadPosition.at(channel), static_cast<float>(mBentBufferLength));
                // increment wrap timing counter
                ++randomLooperPlayCounter.at(channel);
                randomLooperPlayCounter.at(channel) %= randomLooperPlayLength.at(channel);
            }
            // tape FX only
            else
            {
                channelData[sample] = mCircularBuffer.readSample(channel, mReadPosition.at(channel));
                
                mReadPosition.at(channel) += mPlaybackRate.at(channel);
                mReadPosition.at(channel) = wrap(mReadPosition.at(channel), static_cast<float>(mBentBufferLength));
            }
        }
    }
}

void BrokenPlayer::reset()
{    
    for (int line = 0; line < tapeSpeedLine.size(); ++line)
    {
        tapeSpeedLine.at(line).reset(getSampleRate());
        tapeStopLine.at(line).reset(getSampleRate());
    }
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

void BrokenPlayer::setClockSpeed(float newClockSpeed) { clockPeriod = newClockSpeed; }
void BrokenPlayer::setAnalogFX(float newAnalogFX)
{
    tapeBendProb = newAnalogFX;
    
    if (newAnalogFX == 0)
        tapeBendDepth = 0;
    else if (newAnalogFX > 0 && newAnalogFX < 0.35)
        tapeBendDepth = 2;
    else
        tapeBendDepth = 4;
    
    tapeRevProb = std::clamp<float>(newAnalogFX * 1.5, 0.0, 0.8, [](const float& a, const float& b) { return a < b; });
    
    tapeStopProb = 0.4 * pow(newAnalogFX, 1.5);
}
void BrokenPlayer::setDigitalFX(float newDigitalFX)
{
    cdSkipProb = newDigitalFX * 0.35;
    randomLoopProb = newDigitalFX;
}
void BrokenPlayer::setLofiFX(float newLofiFX) {}

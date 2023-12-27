/*
  ==============================================================================

    Broken player classes' implementations
 
  ==============================================================================
*/

#include "BrokenPlayer.h"

// random loop generator
//==============================================================================
RandomLoop::RandomLoop() = default;

RandomLoop::RandomLoop(int bufferLen, int countLen)
:mBufferLength(bufferLen), mCountLength(countLen){}

void RandomLoop::init() { mCounter = 0; }

std::vector<int> RandomLoop::advanceCtrAndReturn()
{
    if (mCounter == 0)
    {
        for (int i = 0; i < 2; ++i)
            mLoopValues.at(i) = (randomFloat() * 64) * ((mBufferLength - 1) / 64);
        
        // normal distribution to 2 std dev - was supposed to make more
        // values close together but actually seemed to make glitches sparser
        
//            std::random_device rd {};
//            std::mt19937 gen { rd() }; // mersenne "twister" random generator
//
//            for (int i = 0; i < 2; ++i)
//            {
//                std::normal_distribution<float> dist { static_cast<float>((bufferLength - 1) / 2), static_cast<float>((bufferLength - 1) / 4) };
//
//                int randomVal = static_cast<int>(std::round(dist(gen)));
//
//                loopValues.at(i) = std::clamp<int>(randomVal, 0, (bufferLength - 1), std::less<int>());
//            }
        
        std::sort(mLoopValues.begin(), mLoopValues.end(), std::less<int>());
    }
    
    ++mCounter;
    mCounter %= mCountLength;
    
    return mLoopValues;
}

void RandomLoop::setBufferLength(int newBufferLen)
{
    mBufferLength = std::clamp<int>(newBufferLen, 0, 352800, std::less<int>());
    mCounter = 0;
}

// cd skip
//==============================================================================
CDSkip::CDSkip(int bufferLen, int bufferDiv)
:mBufferLength(bufferLen), mBufferDivisions(bufferDiv)
{
    mBufferSegmentLength = mBufferLength / mBufferDivisions;
}

void CDSkip::init()
{
    mCounter = 0;
    mSegmentCounter = 0; //?
}

std::vector<int> CDSkip::advanceCtrAndReturn()
{
    if (mCounter == 0)
    {
        // new number of skips
        //countLength = rand() % 3 + 2;
        
        // increment to next segment
        ++mSegmentCounter;
        mSegmentCounter %= (mBufferDivisions - 1);
        
        int newSegmentStart = mSegmentCounter * mBufferSegmentLength;
        
        mLoopValues.at(0) = newSegmentStart >= 0 && newSegmentStart < mBufferLength ? newSegmentStart : 0;
        mLoopValues.at(1) = mBufferSegmentLength >= 4 ? mBufferSegmentLength : 4;
    }
    
    // count number of skips
    ++mCounter;
    mCounter %= mBufferDivisions;
    
    // limit skip end value
    std::for_each(mLoopValues.begin(),
                  mLoopValues.end(),
                  [this](int& value) { value = (value >= mBufferLength ? mBufferLength - 1 : value); });
    
    return mLoopValues;
}

void CDSkip::setBufferLength(int newBufferLen)
{
    mBufferLength = std::clamp<int>(newBufferLen, 0, 352800, std::less<int>());
    mCounter = 0;
    mSegmentCounter = 0;
}

void CDSkip::setBufferDivisions(int newNumDivisions)
{
    mBufferDivisions = newNumDivisions;
    mBufferSegmentLength = mBufferLength / mBufferDivisions;
}

// broken player
//==============================================================================
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
    
    std::for_each(mTapeSpeedLine.begin(),
                  mTapeSpeedLine.end(),
                  [this](auto& line)
    {
        line.setParameters(mRampTime);
        line.reset(getSampleRate());
    });
    
    std::for_each(mTapeStopLine.begin(),
                  mTapeStopLine.end(),
                  [this](auto& line)
    {
        line.setParameters(mRampTime);
        line.reset(getSampleRate());
    });
    
    srand(static_cast<uint32_t>(time(NULL)));
}

//==============================================================================
void BrokenPlayer::releaseResources() {}

//==============================================================================
void BrokenPlayer::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    //================ channel/sample loops ================
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        
        mCircularBuffer.fillNextBlock(channel, buffer.getNumSamples(), channelData);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            //================ clock, clocked settings ================
            if (mShouldUseExternalClock == false)
            {
                if (channel == 0)
                {
                    if (mClockCounter == 0)
                    {
                        receiveClockedPulse();
                        ++mClockCounter;
                    }
                    else
                    {
                        ++mClockCounter;
                        mClockCounter %= mClockCycle;
                    }
                }
            }
            
            //================ tape speed adjustments ================
            // ramp back up if stop completed
            float tapeStopSpeed = mTapeStopLine.at(channel).renderAudioOutput();
            if (tapeStopSpeed < 0.01)
            {
                mTapeStopLine.at(channel).setParameters(133);
                mTapeStopLine.at(channel).setDestination(1.0f);
            }
            
            mPlaybackRate.at(channel) = mTapeSpeedLine.at(channel).renderAudioOutput() * mTapeDirMultiplier;
            mPlaybackRate.at(channel) *= tapeStopSpeed;
            
            //================ playback ================
            //================ repeats ================
            if (mNumRepeats > 1)
            {
                if (channel == 0 && mRepeatsCounter.at(channel) == 0)
                    mRepeatsValues = mRepeater.advanceCtrAndReturn();
                if (mRepeatsCounter.at(channel) == 0)
                    mReadPosition.at(channel) = mRepeatsValues.at(0);
            }
            
            //================ old tape skips ================
//            if (skipProb.at(channel) < cdSkipProb)
//            {
//                std::vector<int> skipLoop;
//
//                // read (and start next slice if necessary) before incrementing and wrapping
//                if (cdSkipPlayCounter.at(channel) == 0)
//                {
//                    skipLoop = cdSkipper.at(channel).advanceCtrAndReturn();
//
//                    mReadPosition.at(channel) = skipLoop.at(0);
//                    mChirpReadPosition.at(channel) = skipLoop.at(0);
//                    cdSkipPlayLength.at(channel) = skipLoop.at(1);
//
//                    channelData[sample] = mCircularBuffer.readSample(channel, mReadPosition.at(channel));
//                }
//                else
//                {
//                    channelData[sample] = mCircularBuffer.readSample(channel, mReadPosition.at(channel));
//                }
//
//                // chirp
//                if (cdSkipPlayCounter.at(channel) < 75)
//                {
//                    channelData[sample] += mCircularBuffer.readSample(channel, mChirpReadPosition.at(channel));
//
//                    mChirpReadPosition.at(channel) += 7;
//                    mChirpReadPosition.at(channel) = wrap(mChirpReadPosition.at(channel), static_cast<float>(mBentBufferLength));
//                }
//
//                // increment/wrap read position
//                mReadPosition.at(channel) += mPlaybackRate.at(channel);
//                mReadPosition.at(channel) = wrap(mReadPosition.at(channel), static_cast<float>(mBentBufferLength));
//                // increment/wrap skip timing counter
//                ++cdSkipPlayCounter.at(channel);
//                cdSkipPlayCounter.at(channel) %= cdSkipPlayLength.at(channel);
//            }

            //================ loops ================
            if (mSkipProb.at(channel) < mRandomLoopProb)
            {
                std::vector<int> randomLoop = mRandomLooper.at(channel).advanceCtrAndReturn();
                
                channelData[sample] = mCircularBuffer.readSample(channel, mReadPosition.at(channel));
                
                // increment/wrap read position
                mReadPosition.at(channel) += mPlaybackRate.at(channel);
                mReadPosition.at(channel) = wrap(mReadPosition.at(channel), static_cast<float>(mBentBufferLength));
                if (mReadPosition.at(channel) > randomLoop.at(1) )//|| mReadPosition.at(channel) < randomLoop.at(0))
                    mReadPosition.at(channel) = randomLoop.at(0);
            }
            //================ advance, only loop at buffer seg. ================
            else
            {
                channelData[sample] = mCircularBuffer.readSample(channel, mReadPosition.at(channel));
                
                mReadPosition.at(channel) += mPlaybackRate.at(channel);
                mReadPosition.at(channel) = wrap(mReadPosition.at(channel), static_cast<float>(mBentBufferLength));
            }
            
            // wrap repeats counter/value
            if (mNumRepeats > 1)
            {
                ++mRepeatsCounter.at(channel);
                if (mRepeatsCounter.at(channel) >= mRepeatsValues.at(1))
                    mRepeatsCounter.at(channel) = 0;
            }
            
        } // end sample loop
    } // end channel loop
    
    // new distortion processor, if necessary
    if (mCurrentDist != mPrevDist)
    {
        mSlotProcessor = mDistortionFactory.create(mCurrentDist);
        
        if (mSlotProcessor != nullptr)
        {
            juce::dsp::ProcessSpec spec;
            spec.sampleRate = getSampleRate();
            spec.maximumBlockSize = buffer.getNumSamples();
            spec.numChannels = buffer.getNumChannels();
            
            mSlotProcessor->prepare(spec);
        }
        
        mPrevDist = mCurrentDist;
    }
    
    // apply correct distortion
    if (mUseDist > 0)
    {
        if (mSlotProcessor != nullptr)
        {
            mSlotProcessor->processBlock(buffer, midiMessages);
        }
    }
}

//==============================================================================
void BrokenPlayer::reset()
{    
    for (int line = 0; line < mTapeSpeedLine.size(); ++line)
    {
        mTapeSpeedLine.at(line).reset(getSampleRate());
        mTapeStopLine.at(line).reset(getSampleRate());
    }
}

//==============================================================================
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

//==============================================================================
void BrokenPlayer::receiveClockedPulse()
{
    //================ L/R tape speed destinations ================
    std::for_each(mTapeSpeedLine.begin(),
                  mTapeSpeedLine.end(),
                  [this](Line<float>& line)
                  {
        if (randomFloat() < mTapeBendProb)
        {
            int index = rand() % mTapeBendDepth;
            float dest = mTapeBendVals.at(index);
            line.setDestination(dest);
        }
    });
    
    if (randomFloat() < mTapeRevProb)
        mTapeDirMultiplier = -1;
    else
        mTapeDirMultiplier = 1;
    
    //================ L/R tape stops ================
    std::for_each(mTapeStopLine.begin(),
                  mTapeStopLine.end(),
                  [this](Line<float>& line)
                  {
        if (randomFloat() < mTapeStopProb)
        {
            line.setParameters(mRampTime);
            line.setDestination(0);
        }
    });
    
    //================ skip/loop probs ================
    std::for_each(mSkipProb.begin(),
                  mSkipProb.end(),
                  [this](float& prob){ prob = randomFloat(); });
    
    //================ distortion FX ================
    // dist
    mUseDist = (randomFloat() < std::clamp<float>(mDistortionProb * 3, 0.0, 1.0, [](const float& a, const float& b) { return a < b; }));
    
    float scaledProb = powf(mDistortionProb, 3.0f);
    
    if (mSlotProcessor != nullptr)
    {
        mDistortionParameters = mSlotProcessor->getParameters();
        
        mDistortionParameters.bitDepth = static_cast<int>(floor( scale(scaledProb * -1 + 1, 0.0f, 1.0f, 5.0f, 12.0f) + 0.5) + (randomFloat() * 3));
        
        mDistortionParameters.downsampling = static_cast<int>( scale(scaledProb, 0.0f, 1.0f, 2.0f, 15.0f) + (randomFloat() * (1 + (scaledProb * 16))) );
        
        mDistortionParameters.drive = scale(mDistortionProb, 0.0f, 1.0f, 3.0f, 15.0f) + (randomFloat() * mDistortionProb * 21.0f);
        
        mSlotProcessor->setParameters(mDistortionParameters);
    }
}
//==============================================================================
void BrokenPlayer::setAnalogFX(float newAnalogFX)
{
    mTapeBendProb = newAnalogFX;
    
    if (newAnalogFX == 0)
    {
        mTapeBendDepth = 0;
        std::for_each(mTapeSpeedLine.begin(),
                      mTapeSpeedLine.end(),
                      [this](Line<float>& line) { line.setDestination(1.0f); });
    }
    else if (newAnalogFX > 0 && newAnalogFX < 0.35)
        mTapeBendDepth = 2;
    else
        mTapeBendDepth = 4;
    
    mTapeRevProb = std::clamp<float>(newAnalogFX * 1.5, 0.0, 0.8, std::less<float>());
    
    mTapeStopProb = 0.4 * powf(newAnalogFX, 1.5);
}
void BrokenPlayer::setDigitalFX(float newDigitalFX)
{
    mRandomLoopProb = powf(newDigitalFX, 0.707f);
}
void BrokenPlayer::setLofiFX(float newLofiFX) { mDistortionProb = newLofiFX; }
void BrokenPlayer::setDistortionType(int newDist) { mCurrentDist = newDist; }
void BrokenPlayer::setBufferLength(int newBufferLength)
{
    mBentBufferLength = std::clamp<int>(newBufferLength, 0, 352800, std::less<int>());
    
    std::for_each(mRandomLooper.begin(),
                  mRandomLooper.end(),
                  [&newBufferLength](RandomLoop& looper)
    {
        looper.setBufferLength(std::clamp<int>(newBufferLength, 0, 352800, std::less<int>()));
    });
    /*
    std::for_each(cdSkipper.begin(),
                  cdSkipper.end(),
                  [&newBufferLength](CDSkip& skipper)
    {
     */
    mRepeater.setBufferLength(std::clamp<int>(newBufferLength, 0, 352800, std::less<int>()));
    //});
}
void BrokenPlayer::newNumRepeats(int newRepeatCount)
{
    /*
    std::for_each(cdSkipper.begin(),
                  cdSkipper.end(),
                  [&newRepeatCount](CDSkip& skipper)
    {
     */
    mRepeater.setBufferDivisions(newRepeatCount);
    mNumRepeats = newRepeatCount;
    //});
}
void BrokenPlayer::setClockSpeed(int newClockSpeed) { mClockCycle = newClockSpeed; }
//void BrokenPlayer::setClockSpeed(float newClockSpeed) { clockPeriod = newClockSpeed; }
void BrokenPlayer::useExternalClock(bool newShouldUseExternalClock) { mShouldUseExternalClock = newShouldUseExternalClock; }

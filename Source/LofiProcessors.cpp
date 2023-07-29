/*
  ==============================================================================

    

  ==============================================================================
*/

// Mu-law encoding, copyright 2014 Emilie Gillet.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//

#include "LofiProcessors.h"

void Bitcrusher::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
}

void Bitcrusher::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto&& inBlock = context.getInputBlock();
    auto&& outBlock = context.getOutputBlock();
    
    jassert(inBlock.getNumChannels() == outBlock.getNumChannels());
    jassert(inBlock.getNumSamples() == outBlock.getNumSamples());
    
    size_t len = inBlock.getNumSamples();
    size_t numChannels = inBlock.getNumChannels();
    
    for (size_t channel = 0; channel < numChannels; ++channel)
    {
        auto* src = inBlock.getChannelPointer(channel);
        auto* dst = outBlock.getChannelPointer(channel);
        
        for (size_t sample = 0; sample < len; ++sample)
        {
            // reduce bit depth
            float totalQLevels = powf(2.0f, bitDepth);
            float val = src[sample];
            float remainder = fmodf(val, 1.0f/totalQLevels);
            
            // quantize
            dst[sample] = val - remainder;
            
            if (downsampling > 1)
            {
                if (sample % downsampling != 0) dst[sample] = dst[sample - (sample % downsampling)];
            }
        }
    }
}

void Bitcrusher::reset()
{
    
}

void Bitcrusher::setBitDepth(float newDepth) { bitDepth = newDepth; }

void Bitcrusher::setDownsampling(int newDownsampling) { downsampling = newDownsampling; }


//==============================================================================
void MuLaw::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
}

void MuLaw::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto&& inBlock = context.getInputBlock();
    auto&& outBlock = context.getOutputBlock();
    
    jassert(inBlock.getNumSamples() == outBlock.getNumSamples());
    jassert(inBlock.getNumChannels() == outBlock.getNumChannels());
    
    size_t numSamples = inBlock.getNumSamples();
    size_t numChannels = inBlock.getNumChannels();
    
    for (size_t channel = 0; channel < numChannels; ++channel)
    {
        auto* src = inBlock.getChannelPointer(channel);
        auto* dst = outBlock.getChannelPointer(channel);
        
        for (size_t sample = 0; sample < numSamples; ++sample)
        {
            float input = src[sample];
            int16_t pcm_in = static_cast<int16_t>(input * 32767.0);
            uint8_t compressed = Lin2MuLaw(pcm_in);
            
            int16_t pcm_out = MuLaw2Lin(compressed);
            dst[sample] = static_cast<float>(pcm_out) * mOutScale;
        }
    }
}

void MuLaw::reset()
{
    
}

inline unsigned char MuLaw::Lin2MuLaw(int16_t pcm_val)
{
    int16_t mask;
    int16_t seg;
    uint8_t uval;
    
    pcm_val = pcm_val >> 2;
    if (pcm_val < 0)
    {
        pcm_val = -pcm_val;
        mask = 0x7f;
    }
    else
    {
        mask = 0xff;
    }
    
    if (pcm_val > 8159) pcm_val = 8159;
    pcm_val += (0x84 >> 2);
    
    if (pcm_val <= 0x3f) seg = 0;
    else if (pcm_val <= 0x7f) seg = 1;
    else if (pcm_val <= 0xff) seg = 2;
    else if (pcm_val <= 0x1ff) seg = 3;
    else if (pcm_val <= 0x3ff) seg = 4;
    else if (pcm_val <= 0x7ff) seg = 5;
    else if (pcm_val <= 0xfff) seg = 6;
    else if (pcm_val <= 0x1fff) seg = 7;
    else seg = 8;
    
    if (seg >= 8)
        return static_cast<uint8_t>(0x7f ^ mask);
    else
    {
        uval = static_cast<uint8_t>((seg << 4) | ((pcm_val >> (seg + 1)) & 0x0f));
        return (uval ^ mask);
    }
}

inline short MuLaw::MuLaw2Lin(uint8_t u_val)
{
    int16_t t;
    u_val = ~u_val;
    t = ((u_val & 0x0f) << 3) + 0x84;
    t <<= (static_cast<unsigned>(u_val) & 0x70) >> 4;
    return ((u_val & 0x80) ? (0x84 - t) : (t - 0x84));
}

//==============================================================================
GSMProcessor::~GSMProcessor()
{
    gsmSignalInput = nullptr;
    gsmSignal = nullptr;
    gsmSignalOutput = nullptr;
    delete[] gsmSignalInput;
    delete[] gsmSignal;
    delete[] gsmSignalOutput;
    gsm_destroy(encode);
    gsm_destroy(decode);
}

void GSMProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    
    filterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((sampleRate / downsamplingAmt) * 0.4, sampleRate, 8);
    
    // pre filters
    preFilter1.reset();
    preFilter1.prepare(spec);
    preFilter1.coefficients = filterCoefficientsArray.getObjectPointer(0);
    
    preFilter2.reset();
    preFilter2.prepare(spec);
    preFilter2.coefficients = filterCoefficientsArray.getObjectPointer(1);
    
    preFilter3.reset();
    preFilter3.prepare(spec);
    preFilter3.coefficients = filterCoefficientsArray.getObjectPointer(2);
    
    preFilter4.reset();
    preFilter4.prepare(spec);
    preFilter4.coefficients = filterCoefficientsArray.getObjectPointer(3);
    
    // post filters
    postFilter1.reset();
    postFilter1.prepare(spec);
    postFilter1.coefficients = filterCoefficientsArray.getObjectPointer(0);
    
    postFilter2.reset();
    postFilter2.prepare(spec);
    postFilter2.coefficients = filterCoefficientsArray.getObjectPointer(1);
    
    postFilter3.reset();
    postFilter3.prepare(spec);
    postFilter3.coefficients = filterCoefficientsArray.getObjectPointer(2);
    
    postFilter4.reset();
    postFilter4.prepare(spec);
    postFilter4.coefficients = filterCoefficientsArray.getObjectPointer(3);
    
    reset();
}

void GSMProcessor::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
}

void GSMProcessor::processBuffer(juce::AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    juce::AudioBuffer<float> monoBuffer(numChannels, numSamples);
    monoBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);
    if (numChannels > 1)
    {
        monoBuffer.addFrom(0, 0, buffer, 1, 0, numSamples);
        monoBuffer.applyGain(0.5f);
    }
    auto* src = monoBuffer.getWritePointer(0);
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // resampling filter
        src[sample] = preFilter1.processSample(src[sample]);
        preFilter1.snapToZero();
        src[sample] = preFilter2.processSample(src[sample]);
        preFilter2.snapToZero();
        src[sample] = preFilter3.processSample(src[sample]);
        preFilter3.snapToZero();
        src[sample] = preFilter4.processSample(src[sample]);
        preFilter4.snapToZero();
        
        if (gsmSignalInput != nullptr && gsmSignal != nullptr && gsmSignalOutput != nullptr)
        {
            float currentSample { 0.0f };
            if (downsamplingCounter == 0)
            {
                gsmSignalInput[gsmSignalCounter] = static_cast<gsm_signal>(src[sample] * 4096.0f);
                
                // signal is stored in *highest* 13 bits; lowest 3 are zeroes
                // shift operators not *supposed* to work on neg, but maybe this works bc encoding is unique
                gsmSignalInput[gsmSignalCounter] <<= 3;
                gsmSignalInput[gsmSignalCounter] &= 0b1111111111111000;
                
                // update counter
                ++gsmSignalCounter;
                gsmSignalCounter %= 160;
                
                // gsm signal block into encoder/decoder
                if (gsmSignalCounter == 0)
                {
                    gsmSignal = gsmSignalInput;
                    gsm_encode(encode, gsmSignal, gsmFrame);
                    gsm_decode(decode, gsmFrame, gsmSignal);
                    gsmSignalOutput = gsmSignal;
                }
                
                // signals to output
                gsmSignalOutput[gsmSignalCounter] >>= 3;
                currentSample = static_cast<float>(gsmSignalOutput[gsmSignalCounter]) / 4096.0f;
            }
            
            ++downsamplingCounter;
            if (downsamplingAmt != 0)
                downsamplingCounter %= downsamplingAmt;
            else
                downsamplingCounter = 0;
            
            currentSample = postFilter1.processSample(currentSample);
            postFilter1.snapToZero();
            currentSample = postFilter2.processSample(currentSample);
            postFilter2.snapToZero();
            currentSample = postFilter3.processSample(currentSample);
            postFilter3.snapToZero();
            currentSample = postFilter4.processSample(currentSample);
            postFilter4.snapToZero();
            
            std::vector<float> downsamplingGainComp { 1.0f, 1.0f, 1.45f, 2.35f, 3.5f, 4.35f, 5.5f, 6.25f, 7.0f };
            currentSample *= downsamplingGainComp[downsamplingAmt];
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                auto* dst = buffer.getWritePointer(channel);
                // signals to output
                dst[sample] = currentSample;
            }
        }
    }
}

void GSMProcessor::reset()
{
}

void GSMProcessor::setDownsampling(int newDownsampling)
{
    downsamplingAmt = newDownsampling;
    
    if (downsamplingAmt != prevDownsamplingAmt)
    {
        prevDownsamplingAmt = downsamplingAmt;
        
        // pre filters
        preFilter1.reset();
        preFilter2.reset();
        preFilter3.reset();
        preFilter4.reset();
        
        // post filters
        postFilter1.reset();
        postFilter2.reset();
        postFilter3.reset();
        postFilter4.reset();
        
        if (downsamplingAmt != 0)
            filterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((sampleRate / downsamplingAmt) * 0.4, sampleRate, 8);
        else
            filterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(sampleRate * 0.4, sampleRate, 8);
        
        // pre filters
        preFilter1.coefficients = filterCoefficientsArray.getObjectPointer(0);
        preFilter2.coefficients = filterCoefficientsArray.getObjectPointer(1);
        preFilter3.coefficients = filterCoefficientsArray.getObjectPointer(2);
        preFilter4.coefficients = filterCoefficientsArray.getObjectPointer(3);
        
        // post filters
        postFilter1.coefficients = filterCoefficientsArray.getObjectPointer(0);
        postFilter2.coefficients = filterCoefficientsArray.getObjectPointer(1);
        postFilter3.coefficients = filterCoefficientsArray.getObjectPointer(2);
        postFilter4.coefficients = filterCoefficientsArray.getObjectPointer(3);
    }
}

//==============================================================================
void DownsampleAndFilter::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    
    auto numChannels = spec.numChannels;
    
    preFilter1.resize(numChannels);
    for (auto& ch : preFilter1)
        ch.prepare(spec);
    preFilter2.resize(numChannels);
    for (auto& ch : preFilter2)
        ch.prepare(spec);
    preFilter3.resize(numChannels);
    for (auto& ch : preFilter3)
        ch.prepare(spec);
    preFilter4.resize(numChannels);
    for (auto& ch : preFilter4)
        ch.prepare(spec);
    
    postFilter1.resize(numChannels);
    for (auto& ch : postFilter1)
        ch.prepare(spec);
    postFilter2.resize(numChannels);
    for (auto& ch : postFilter2)
        ch.prepare(spec);
    postFilter3.resize(numChannels);
    for (auto& ch : postFilter3)
        ch.prepare(spec);
    postFilter4.resize(numChannels);
    for (auto& ch : postFilter4)
        ch.prepare(spec);
    
    std::for_each(resamplingRamps.begin(),
                      resamplingRamps.end(),
                      [this](Line<float>& line) {
        line.reset(sampleRate);
        line.setParameters(downsampling);
    });
    
    preFilter1L.prepare(spec);
    preFilter1R.prepare(spec);
    
    reset();
}

void DownsampleAndFilter::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto&& inBlock = context.getInputBlock();
    auto&& outBlock = context.getOutputBlock();
    
    jassert(inBlock.getNumSamples() == outBlock.getNumSamples());
    jassert(inBlock.getNumChannels() == outBlock.getNumChannels());
    
    auto numSamples = inBlock.getNumSamples();
    auto numChannels = inBlock.getNumChannels();
    
    // downsampling
    for (auto channel = 0; channel < numChannels; ++channel)
    {
        auto* src = inBlock.getChannelPointer(channel);
        auto* dst = outBlock.getChannelPointer(channel);
        
//        juce::dsp::AudioBlock<float> channelBlock = outBlock.getSingleChannelBlock(channel);

//        if (channel == 0)
//        {
//            preFilter1L.process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
//            preFilter1L.snapToZero();
//        }
//        else
//        {
//            preFilter1R.process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
//            preFilter1R.snapToZero();
//        }
//        preFilter1[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
//        preFilter1[channel].snapToZero();
//        preFilter2[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
//        preFilter2[channel].snapToZero();
//        preFilter3[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
//        preFilter3[channel].snapToZero();
//        preFilter4[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
//        preFilter4[channel].snapToZero();
        
        
        for (auto sample = 0; sample < numSamples; ++sample)
        {
//            // pre-downsampling filters
//            dst[sample] = preFilter1[channel].processSample(src[sample]);
//            preFilter1[channel].snapToZero();
//            dst[sample] = preFilter2[channel].processSample(dst[sample]);
//            preFilter2[channel].snapToZero();
//            dst[sample] = preFilter3[channel].processSample(dst[sample]);
//            preFilter3[channel].snapToZero();
//            dst[sample] = preFilter4[channel].processSample(dst[sample]);
//            preFilter4[channel].snapToZero();
            
            if (downsampling > 1)
            {
//                float currentSample { 0.0f };
//                if (sample % downsampling == 0) currentSample = dst[sample];
//                dst[sample] = currentSample;
                
                resamplingRamps.at(channel).setDestination(src[sample]);
                dst[sample] = resamplingRamps.at(channel).renderAudioOutput();
            }

//            // post-downsampling images filters
//            dst[sample] = postFilter1[channel].processSample(dst[sample]);
//            postFilter1[channel].snapToZero();
//            dst[sample] = postFilter2[channel].processSample(dst[sample]);
//            postFilter2[channel].snapToZero();
//            dst[sample] = postFilter3[channel].processSample(dst[sample]);
//            postFilter3[channel].snapToZero();
//            dst[sample] = postFilter4[channel].processSample(dst[sample]);
//            postFilter4[channel].snapToZero();
             
        }
        
//        postFilter1[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
//        postFilter1[channel].snapToZero();
//        postFilter2[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
//        postFilter2[channel].snapToZero();
//        postFilter3[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
//        postFilter3[channel].snapToZero();
//        postFilter4[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
//        postFilter4[channel].snapToZero();
    }
}

void DownsampleAndFilter::processBuffer(juce::AudioBuffer<float>& buffer)
{
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    for (auto channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (auto sample = 0; sample < numSamples; ++sample)
        {
            // pre-downsampling filters
            channelData[sample] = preFilter1[channel].processSample(channelData[sample]);
            preFilter1[channel].snapToZero();
            channelData[sample] = preFilter2[channel].processSample(channelData[sample]);
            preFilter2[channel].snapToZero();
            channelData[sample] = preFilter3[channel].processSample(channelData[sample]);
            preFilter3[channel].snapToZero();
            channelData[sample] = preFilter4[channel].processSample(channelData[sample]);
            preFilter4[channel].snapToZero();

            if (downsampling > 1)
            {
//                if (sample % downsampling != 0) channelData[sample] = channelData[sample - (sample % downsampling)];
                float currentSample { 0.0 };
                if (sample % downsampling == 0) currentSample = channelData[sample];
                else channelData[sample] = currentSample;
            }

            // post-downsampling images filters
            channelData[sample] = postFilter1[channel].processSample(channelData[sample]);
            postFilter1[channel].snapToZero();
            channelData[sample] = postFilter2[channel].processSample(channelData[sample]);
            postFilter2[channel].snapToZero();
            channelData[sample] = postFilter3[channel].processSample(channelData[sample]);
            postFilter3[channel].snapToZero();
            channelData[sample] = postFilter4[channel].processSample(channelData[sample]);
            postFilter4[channel].snapToZero();
        }
    }
}

void DownsampleAndFilter::reset()
{
    filterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(cutoff, sampleRate, 8);
    
    preFilter1L.reset();
    preFilter1R.reset();
    
    preFilter1L.coefficients = filterCoefficientsArray.getObjectPointer(0);
    preFilter1R.coefficients = filterCoefficientsArray.getObjectPointer(0);
    
    // input
    for (auto& ch : preFilter1)
    {
        ch.reset();
        ch.coefficients = filterCoefficientsArray.getObjectPointer(0);
    }
    for (auto& ch : preFilter2)
    {
        ch.reset();
        ch.coefficients = filterCoefficientsArray.getObjectPointer(1);
    }
    for (auto& ch : preFilter3)
    {
        ch.reset();
        ch.coefficients = filterCoefficientsArray.getObjectPointer(2);
    }
    for (auto& ch : preFilter4)
    {
        ch.reset();
        ch.coefficients = filterCoefficientsArray.getObjectPointer(3);
    }
    
    // output
    for (auto& ch : postFilter1)
    {
        ch.reset();
        ch.coefficients = filterCoefficientsArray.getObjectPointer(0);
    }
    for (auto& ch : postFilter2)
    {
        ch.reset();
        ch.coefficients = filterCoefficientsArray.getObjectPointer(1);
    }
    for (auto& ch : postFilter3)
    {
        ch.reset();
        ch.coefficients = filterCoefficientsArray.getObjectPointer(2);
    }
    for (auto& ch : postFilter4)
    {
        ch.reset();
        ch.coefficients = filterCoefficientsArray.getObjectPointer(3);
    }
}

void DownsampleAndFilter::setDownsampling(int newDownsampling)
{
    if (newDownsampling <= 0)
        downsampling = 1;
    else if ((newDownsampling % 2) == 0) // x is pwr of 2 when log2(x) == int
        downsampling = newDownsampling;
    else
        downsampling = 1;
    
    cutoff = (sampleRate / downsampling) * 0.4;
    
    std::for_each(resamplingRamps.begin(),
                      resamplingRamps.end(),
                      [this](Line<float>& line) { line.setParameters(downsampling); });
    
    reset();
}


//==============================================================================
void ChebyDrive::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto&& inBlock = context.getInputBlock();
    auto&& outBlock = context.getOutputBlock();
    
    jassert(inBlock.getNumSamples() == outBlock.getNumSamples());
    jassert(inBlock.getNumChannels() == outBlock.getNumChannels());
    
    size_t numSamples = inBlock.getNumSamples();
    size_t numChannels = inBlock.getNumChannels();
    
    // drive
    for (size_t channel = 0; channel < numChannels; ++channel)
    {
        auto* src = inBlock.getChannelPointer(channel);
        auto* dst = outBlock.getChannelPointer(channel);
        
        for (size_t sample = 0; sample < numSamples; ++sample)
        {
            float wetSignal = ((std::pow(2.0f * src[sample], 2.0f) - 1.0f) * std::sin(0.5 * M_PI * drive));
            float drySignal = src[sample] * std::cos(0.5 * M_PI * drive);
            dst[sample] = wetSignal + drySignal;
        }
    }
}

void ChebyDrive::setDrive(float newDrive) { drive = newDrive; }

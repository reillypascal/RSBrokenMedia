/*
  ==============================================================================

 Distortion:
 - Bitcrusher
 - Tanh Saturation
 Codec:
 - MuLaw
 - GSM 06.10
 Removed:
 - Chebyshev Drive
 - Downsample and Filter
 
  ==============================================================================
*/

#include "LofiProcessors.h"

Bitcrusher::Bitcrusher() = default;

Bitcrusher::~Bitcrusher() = default;

void Bitcrusher::prepare(const juce::dsp::ProcessSpec& spec)
{
    mSampleRate = spec.sampleRate;
    reset();
}

void Bitcrusher::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // reduce bit depth
            float totalQLevels = powf(2.0f, parameters.bitDepth);
            float val = channelData[sample];
            float remainder = fmodf(val, 1.0f/totalQLevels);
            
            // quantize
            channelData[sample] = val - remainder;
            
            if (parameters.downsampling > 1)
            {
                if (sample % parameters.downsampling != 0) channelData[sample] = channelData[sample - (sample % parameters.downsampling)];
            }
        }
    }
}

void Bitcrusher::reset() {}

LofiProcessorParameters& Bitcrusher::getParameters() { return parameters; }

void Bitcrusher::setParameters(const LofiProcessorParameters& params)
{
    if (parameters.bitDepth != params.bitDepth || parameters.downsampling != params.downsampling)
    {
        parameters = params;
    }
}

//==============================================================================
SaturationProcessor::SaturationProcessor() : mLowCutFilter(juce::dsp::IIR::Coefficients<float>::makeHighPass(44100, 75.0f)) {}

SaturationProcessor::~SaturationProcessor() = default;

void SaturationProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    mSampleRate = spec.sampleRate;
    mNumChannels = spec.numChannels;
    
    mLowCutFilter.reset();
    mLowCutFilter.prepare(spec);
    
//    mLowCutFilterCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(mSampleRate, 75.0, 0.707);
    
    reset();
}

void SaturationProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    float saturationGainLin = pow(10.0f, parameters.drive / 20.0f);
    
    juce::dsp::AudioBlock<float> block(buffer);
    mLowCutFilter.process(juce::dsp::ProcessContextReplacing<float> (block));
    
    // drive
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] = std::tanh(saturationGainLin * channelData[sample]);
            channelData[sample] *= 0.5f;
        }
    }
}

void SaturationProcessor::reset() {}

LofiProcessorParameters& SaturationProcessor::getParameters() { return parameters; }

void SaturationProcessor::setParameters(const LofiProcessorParameters& params)
{
    if (parameters.drive != params.drive)
    {
        parameters.drive = params.drive;
        parameters = params;
    }
}

float SaturationProcessor::softClip(float x)
{
    if (x > 3.0f)
        return 1.0f;
    else if (x < -3.0f)
        return -1.0f;
    else return x * (27.0 + x * x) / (27.0 + 9.0 * x * x);
}

//==============================================================================
MuLawProcessor::MuLawProcessor() = default;

MuLawProcessor::~MuLawProcessor() = default;

void MuLawProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    mSampleRate = spec.sampleRate;
    mNumChannels = spec.numChannels;
    
    mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / parameters.downsampling) * 0.4, mSampleRate, mResamplingFilterOrder);
        
    preFilters.resize(mNumChannels);
    postFilters.resize(mNumChannels);
    
    for (int channel = 0; channel < mNumChannels; ++channel)
    {
        preFilters[channel].resize(mResamplingFilterOrder / 2);
        postFilters[channel].resize(mResamplingFilterOrder / 2);
        
        for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
        {
            preFilters[channel][filter].reset();
            preFilters[channel][filter].prepare(spec);
            preFilters[channel][filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
            
            postFilters[channel][filter].reset();
            postFilters[channel][filter].prepare(spec);
            postFilters[channel][filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
        }
    }
    
    reset();
}

void MuLawProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Mu-Law processing on channelData
            int16_t pcm_in = static_cast<int16_t>(channelData[sample] * 32767.0);
            uint8_t compressed = Lin2MuLaw(pcm_in);
            
            int16_t pcm_out = MuLaw2Lin(compressed);
            channelData[sample] = static_cast<float>(pcm_out) * mOutScale;
            
            // downsample and filter
            if (parameters.downsampling > 1)
            {
                // pre-filtering
                for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
                {
                    channelData[sample] = preFilters[channel][filter].processSample(channelData[sample]);
                    preFilters[channel][filter].snapToZero();
                }
                
                // downsampling
                if (mDownsamplingCounter[channel] == 0)
                    mDownsamplingInput[channel] = channelData[sample];
                
                channelData[sample] = mDownsamplingInput[channel];
                
                ++mDownsamplingCounter[channel];
                mDownsamplingCounter[channel] %= parameters.downsampling;
                
                // post-filtering
                for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
                {
                    channelData[sample] = postFilters[channel][filter].processSample(channelData[sample]);
                    postFilters[channel][filter].snapToZero();
                }
                
//                std::vector<float> downsamplingGainComp { 1.0f, 1.0f, 1.45f, 2.35f, 3.5f, 4.35f, 5.5f, 6.25f, 7.0f };
//                channelData[sample] *= downsamplingGainComp[parameters.downsampling];
            }
        }
    }
}

void MuLawProcessor::reset() {}

LofiProcessorParameters& MuLawProcessor::getParameters() { return parameters; }

void MuLawProcessor::setParameters(const LofiProcessorParameters& params)
{
    if (parameters.downsampling != params.downsampling)
    {
        // coefficients
        mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / params.downsampling) * 0.4, mSampleRate, mResamplingFilterOrder); 
        
        for (int channel = 0; channel < mNumChannels; ++channel)
        {
            for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
            {
                preFilters[channel][filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
                
                postFilters[channel][filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
            }
        }
    }
    
    parameters = params;
}

inline unsigned char MuLawProcessor::Lin2MuLaw(int16_t pcm_val)
{
    int sign = (pcm_val >> 8) & 0x80;
    if (sign)
        pcm_val = static_cast<int16_t>(-pcm_val);
    if (pcm_val > cClip)
        pcm_val = cClip;
    pcm_val = static_cast<int16_t>(pcm_val + cBias);
    int exponent = static_cast<int>(MuLawCompressTable[(pcm_val >> 7) & 0xff]);
    int mantissa = (pcm_val >> (exponent + 3)) & 0x0f;
    int compressedByte = ~(sign | (exponent << 4) | mantissa);
    
    return static_cast<unsigned char>(compressedByte);
}

inline short MuLawProcessor::MuLaw2Lin(uint8_t u_val)
{
    return MuLawDecompressTable[u_val];
}

//==============================================================================
GSMProcessor::GSMProcessor() = default;

GSMProcessor::~GSMProcessor() = default;

void GSMProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    mSampleRate = spec.sampleRate;
    
    mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / parameters.downsampling) * 0.4, mSampleRate, mResamplingFilterOrder);
    
    mPreFilters.resize(mResamplingFilterOrder / 2);
    mPostFilters.resize(mResamplingFilterOrder / 2);
    
    for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
    {
        // prepare each pre-filter
        mPreFilters[filter].reset();
        mPreFilters[filter].prepare(spec);
        mPreFilters[filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
        
        mPostFilters[filter].reset();
        mPostFilters[filter].prepare(spec);
        mPostFilters[filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
    }
    
    reset();
}

void GSMProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    juce::AudioBuffer<float> monoBuffer(1, numSamples);
    monoBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);
    if (numChannels > 1)
    {
        monoBuffer.addFrom(0, 0, buffer, 1, 0, numSamples);
        monoBuffer.applyGain(0.5f);
    }
    
    auto* src = monoBuffer.getWritePointer(0);
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // if gsm data variables are valid, process audio
        if (mGsmSignalInput != nullptr && mGsmSignal != nullptr && mGsmSignalOutput != nullptr)
        {
            // ================ pre-filtering block ================
            // low cut filter
//            src[sample] = mLowCutFilter.processSample(src[sample]);
            
            // pre-filter if downsampling
            if (parameters.downsampling > 1)
            {
                for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
                {
                    src[sample] = mPreFilters[filter].processSample(src[sample]);
                    mPreFilters[filter].snapToZero();
                }
            }
            
            //================ GSM processing block ================
            // sync data rate to downsampling counter
            if (mDownsamplingCounter == 0)
            {
                mGsmSignalInput.get()[mGsmSignalCounter] = static_cast<gsm_signal>(src[sample] * 4096.0f);
                
                mGsmSignalInput.get()[mGsmSignalCounter] <<= 3;
                mGsmSignalInput.get()[mGsmSignalCounter] &= 0b1111111111111000;
                
                ++mGsmSignalCounter;
                mGsmSignalCounter %= 160;
                
                if (mGsmSignalCounter == 0)
                {
                    std::swap(mGsmSignalInput, mGsmSignal);
                    gsm_encode(mEncode.get(), mGsmSignal.get(), mGsmFrame.get());
                    gsm_decode(mDecode.get(), mGsmFrame.get(), mGsmSignal.get());
                    std::swap(mGsmSignal, mGsmSignalOutput);
                }
                
                mGsmSignalOutput.get()[mGsmSignalCounter] >>= 3;
                // sample has moved from src -> gsm -> currentSample
                mCurrentSample = static_cast<float>(mGsmSignalOutput.get()[mGsmSignalCounter]) / 4096.0f;
            }
            // return sample to src for filtering
            src[sample] = mCurrentSample;
            
            // increment downsampling frame
            ++mDownsamplingCounter;
            mDownsamplingCounter %= parameters.downsampling;
            
            //================= post-filtering block =================
            // post-filter and compensate if downsampling
            if (parameters.downsampling > 1)
            {
                // post-filtering
                for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
                {
                    src[sample] = mPostFilters[filter].processSample(src[sample]);
                    mPostFilters[filter].snapToZero();
                }
//                std::vector<float> downsamplingGainComp { 1.0f, 1.0f, 1.45f, 2.35f, 3.5f, 4.35f, 5.5f, 6.25f, 7.0f };
//                src[sample] *= downsamplingGainComp[parameters.downsampling];
            }
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                auto* dst = buffer.getWritePointer(channel);
                // signals to output
                dst[sample] = src[sample];
            }
        }
    }
}

void GSMProcessor::reset() {}

LofiProcessorParameters& GSMProcessor::getParameters() { return parameters; }

void GSMProcessor::setParameters(const LofiProcessorParameters& params)
{
    if (parameters.downsampling != params.downsampling)
    {
        // coefficients
        mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / params.downsampling) * 0.4, mSampleRate, mResamplingFilterOrder);
        
        for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
        {
            // update each pre-filter
            mPreFilters[filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
            
            // update each post-filter
            mPostFilters[filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
        }
    }
    
    parameters = params;
}

//==============================================================================
//ChebyDrive::ChebyDrive() = default;
//
//ChebyDrive::~ChebyDrive() = default;
//
//void ChebyDrive::prepare(const juce::dsp::ProcessSpec & spec) {}
//    
//void ChebyDrive::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
//{
//    int numSamples = buffer.getNumSamples();
//    int numChannels = buffer.getNumChannels();
//    
//    float saturationGainLin = pow(10.0f, parameters.drive / 20.0f);
//    
//    // drive
//    for (int channel = 0; channel < numChannels; ++channel)
//    {
//        auto* channelData = buffer.getWritePointer(channel);
//        
//        for (int sample = 0; sample < numSamples; ++sample)
//        {
////            float wetSignal = ((std::pow(2.0f * channelData[sample], 2.0f) - 1.0f) * std::sin(0.5 * M_PI * parameters.drive));
////            float drySignal = channelData[sample] * std::cos(0.5 * M_PI * parameters.drive);
////            channelData[sample] = wetSignal + drySignal;
//            
//            float chebySignal = ((std::pow(2.0f * channelData[sample], 2.0f) - 1.0f) * std::sin(0.5 * M_PI * parameters.drive));
//            float driveSignal = tanh(chebySignal * saturationGainLin);
//            channelData[sample] = driveSignal;
//        }
//    }
//}
//
//void ChebyDrive::reset() {}
//
//LofiProcessorParameters& ChebyDrive::getParameters() { return parameters; }
//
//void ChebyDrive::setParameters(const LofiProcessorParameters& params)
//{
//    if (parameters.drive != params.drive)
//    {
//        parameters.drive = params.drive;
//        parameters = params;
//    }
//}

//==============================================================================
//void DownsampleAndFilter::prepare(const juce::dsp::ProcessSpec& spec)
//{
//    sampleRate = spec.sampleRate;
//    
//    auto numChannels = spec.numChannels;
//    
//    preFilter1.resize(numChannels);
//    for (auto& ch : preFilter1)
//        ch.prepare(spec);
//    preFilter2.resize(numChannels);
//    for (auto& ch : preFilter2)
//        ch.prepare(spec);
//    preFilter3.resize(numChannels);
//    for (auto& ch : preFilter3)
//        ch.prepare(spec);
//    preFilter4.resize(numChannels);
//    for (auto& ch : preFilter4)
//        ch.prepare(spec);
//    
//    postFilter1.resize(numChannels);
//    for (auto& ch : postFilter1)
//        ch.prepare(spec);
//    postFilter2.resize(numChannels);
//    for (auto& ch : postFilter2)
//        ch.prepare(spec);
//    postFilter3.resize(numChannels);
//    for (auto& ch : postFilter3)
//        ch.prepare(spec);
//    postFilter4.resize(numChannels);
//    for (auto& ch : postFilter4)
//        ch.prepare(spec);
//    
//    std::for_each(resamplingRamps.begin(),
//                      resamplingRamps.end(),
//                      [this](Line<float>& line) {
//        line.reset(sampleRate);
//        line.setParameters(downsampling);
//    });
//    
////    preFilter1L.prepare(spec);
////    preFilter1R.prepare(spec);
//    
//    downsamplingCounter.resize(numChannels, 0);
//    currentSample.resize(numChannels, 0.0f);
//    
//    reset();
//}
//
//void DownsampleAndFilter::process(const juce::dsp::ProcessContextReplacing<float>& context)
//{
//    auto&& inBlock = context.getInputBlock();
//    auto&& outBlock = context.getOutputBlock();
//    
//    jassert(inBlock.getNumSamples() == outBlock.getNumSamples());
//    jassert(inBlock.getNumChannels() == outBlock.getNumChannels());
//    
//    auto numSamples = inBlock.getNumSamples();
//    auto numChannels = inBlock.getNumChannels();
//    
//    // downsampling
//    for (auto channel = 0; channel < numChannels; ++channel)
//    {
//        auto* src = inBlock.getChannelPointer(channel);
//        auto* dst = outBlock.getChannelPointer(channel);
//        
////        juce::dsp::AudioBlock<float> channelBlock = outBlock.getSingleChannelBlock(channel);
//
////        if (channel == 0)
////        {
////            preFilter1L.process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
////            preFilter1L.snapToZero();
////        }
////        else
////        {
////            preFilter1R.process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
////            preFilter1R.snapToZero();
////        }
////        preFilter1[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
////        preFilter1[channel].snapToZero();
////        preFilter2[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
////        preFilter2[channel].snapToZero();
////        preFilter3[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
////        preFilter3[channel].snapToZero();
////        preFilter4[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
////        preFilter4[channel].snapToZero();
//        
//        
//        for (auto sample = 0; sample < numSamples; ++sample)
//        {
//            // pre-downsampling filters
//            dst[sample] = preFilter1[channel].processSample(src[sample]);
//            preFilter1[channel].snapToZero();
//            dst[sample] = preFilter2[channel].processSample(dst[sample]);
//            preFilter2[channel].snapToZero();
//            dst[sample] = preFilter3[channel].processSample(dst[sample]);
//            preFilter3[channel].snapToZero();
//            dst[sample] = preFilter4[channel].processSample(dst[sample]);
//            preFilter4[channel].snapToZero();
//            
//            if (downsampling > 1)
//            {
//                if (downsamplingCounter[channel] == 0)
//                    currentSample[channel] = dst[sample];
//                dst[sample] = currentSample[channel];
//                
//                ++downsamplingCounter[channel];
//                if (downsampling != 0)
//                    downsamplingCounter[channel] %= downsampling;
//                else
//                    downsamplingCounter[channel] = 0;
//                
////                resamplingRamps.at(channel).setDestination(src[sample]);
////                dst[sample] = resamplingRamps.at(channel).renderAudioOutput();
//            }
//
//            // post-downsampling images filters
//            dst[sample] = postFilter1[channel].processSample(dst[sample]);
//            postFilter1[channel].snapToZero();
//            dst[sample] = postFilter2[channel].processSample(dst[sample]);
//            postFilter2[channel].snapToZero();
//            dst[sample] = postFilter3[channel].processSample(dst[sample]);
//            postFilter3[channel].snapToZero();
//            dst[sample] = postFilter4[channel].processSample(dst[sample]);
//            postFilter4[channel].snapToZero();
//             
//        }
//        
////        postFilter1[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
////        postFilter1[channel].snapToZero();
////        postFilter2[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
////        postFilter2[channel].snapToZero();
////        postFilter3[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
////        postFilter3[channel].snapToZero();
////        postFilter4[channel].process(juce::dsp::ProcessContextReplacing<float> { channelBlock });
////        postFilter4[channel].snapToZero();
//    }
//}
//
//void DownsampleAndFilter::processBuffer(juce::AudioBuffer<float>& buffer)
//{
//    auto numSamples = buffer.getNumSamples();
//    auto numChannels = buffer.getNumChannels();
//
//    for (auto channel = 0; channel < numChannels; ++channel)
//    {
//        auto* channelData = buffer.getWritePointer(channel);
//
//        for (auto sample = 0; sample < numSamples; ++sample)
//        {
//            // pre-downsampling filters
//            channelData[sample] = preFilter1[channel].processSample(channelData[sample]);
//            preFilter1[channel].snapToZero();
//            channelData[sample] = preFilter2[channel].processSample(channelData[sample]);
//            preFilter2[channel].snapToZero();
//            channelData[sample] = preFilter3[channel].processSample(channelData[sample]);
//            preFilter3[channel].snapToZero();
//            channelData[sample] = preFilter4[channel].processSample(channelData[sample]);
//            preFilter4[channel].snapToZero();
//
//            if (downsampling > 1)
//            {
////                if (sample % downsampling != 0) channelData[sample] = channelData[sample - (sample % downsampling)];
//                float currentSample { 0.0 };
//                if (sample % downsampling == 0) currentSample = channelData[sample];
//                else channelData[sample] = currentSample;
//            }
//
//            // post-downsampling images filters
//            channelData[sample] = postFilter1[channel].processSample(channelData[sample]);
//            postFilter1[channel].snapToZero();
//            channelData[sample] = postFilter2[channel].processSample(channelData[sample]);
//            postFilter2[channel].snapToZero();
//            channelData[sample] = postFilter3[channel].processSample(channelData[sample]);
//            postFilter3[channel].snapToZero();
//            channelData[sample] = postFilter4[channel].processSample(channelData[sample]);
//            postFilter4[channel].snapToZero();
//        }
//    }
//}
//
//void DownsampleAndFilter::reset()
//{
//    filterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(cutoff, sampleRate, 8);
//    
////    preFilter1L.reset();
////    preFilter1R.reset();
////
////    preFilter1L.coefficients = filterCoefficientsArray.getObjectPointer(0);
////    preFilter1R.coefficients = filterCoefficientsArray.getObjectPointer(0);
//    
//    // input
//    for (auto& ch : preFilter1)
//    {
//        ch.reset();
//        ch.coefficients = filterCoefficientsArray.getObjectPointer(0);
//    }
//    for (auto& ch : preFilter2)
//    {
//        ch.reset();
//        ch.coefficients = filterCoefficientsArray.getObjectPointer(1);
//    }
//    for (auto& ch : preFilter3)
//    {
//        ch.reset();
//        ch.coefficients = filterCoefficientsArray.getObjectPointer(2);
//    }
//    for (auto& ch : preFilter4)
//    {
//        ch.reset();
//        ch.coefficients = filterCoefficientsArray.getObjectPointer(3);
//    }
//    
//    // output
//    for (auto& ch : postFilter1)
//    {
//        ch.reset();
//        ch.coefficients = filterCoefficientsArray.getObjectPointer(0);
//    }
//    for (auto& ch : postFilter2)
//    {
//        ch.reset();
//        ch.coefficients = filterCoefficientsArray.getObjectPointer(1);
//    }
//    for (auto& ch : postFilter3)
//    {
//        ch.reset();
//        ch.coefficients = filterCoefficientsArray.getObjectPointer(2);
//    }
//    for (auto& ch : postFilter4)
//    {
//        ch.reset();
//        ch.coefficients = filterCoefficientsArray.getObjectPointer(3);
//    }
//}
//
//void DownsampleAndFilter::setDownsampling(int newDownsampling)
//{
//    if (newDownsampling <= 0)
//        downsampling = 1;
//    else if ((newDownsampling % 2) == 0) // x is pwr of 2 when log2(x) == int
//        downsampling = newDownsampling;
//    else
//        downsampling = 1;
//    
//    cutoff = (sampleRate / downsampling) * 0.4;
//    
//    std::for_each(resamplingRamps.begin(),
//                      resamplingRamps.end(),
//                      [this](Line<float>& line) { line.setParameters(downsampling); });
//    
//    if (downsampling != prevDownsampling)
//    {
//        prevDownsampling = downsampling;
//        filterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(cutoff, sampleRate, 8);
//        
//        // input
//        for (auto& ch : preFilter1)
//        {
//            ch.reset();
//            ch.coefficients = filterCoefficientsArray.getObjectPointer(0);
//        }
//        for (auto& ch : preFilter2)
//        {
//            ch.reset();
//            ch.coefficients = filterCoefficientsArray.getObjectPointer(1);
//        }
//        for (auto& ch : preFilter3)
//        {
//            ch.reset();
//            ch.coefficients = filterCoefficientsArray.getObjectPointer(2);
//        }
//        for (auto& ch : preFilter4)
//        {
//            ch.reset();
//            ch.coefficients = filterCoefficientsArray.getObjectPointer(3);
//        }
//        
//        // output
//        for (auto& ch : postFilter1)
//        {
//            ch.reset();
//            ch.coefficients = filterCoefficientsArray.getObjectPointer(0);
//        }
//        for (auto& ch : postFilter2)
//        {
//            ch.reset();
//            ch.coefficients = filterCoefficientsArray.getObjectPointer(1);
//        }
//        for (auto& ch : postFilter3)
//        {
//            ch.reset();
//            ch.coefficients = filterCoefficientsArray.getObjectPointer(2);
//        }
//        for (auto& ch : postFilter4)
//        {
//            ch.reset();
//            ch.coefficients = filterCoefficientsArray.getObjectPointer(3);
//        }
//    }
//}

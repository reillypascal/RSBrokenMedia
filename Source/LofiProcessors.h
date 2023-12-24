/*
  ==============================================================================

    Make processor base class?

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Modulators.h"
#include "Utilities.h"

// gsm files (in C)
extern "C" {
#include "gsm/config.h"
#include "gsm/gsm.h"
#include "gsm/private.h"
#include "gsm/proto.h"
#include "gsm/unproto.h"
}

class Bitcrusher : public juce::dsp::ProcessorBase
{
public:
    Bitcrusher() = default;
    
    ~Bitcrusher() = default;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    
    void reset() override;
    
    void setBitDepth(float newDepth);
    
    void setDownsampling(int newDownsampling);
    
private:
    int sampleRate = 44100;
    float bitDepth = 8;
    int downsampling = 4;
};

//==============================================================================
class MuLawProcessor : public LofiProcessorBase
{
public:
    MuLawProcessor();
    
    ~MuLawProcessor() override;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void reset() override;
    
    LofiProcessorParameters& getParameters() override;
    
    void setParameters(const LofiProcessorParameters& params) override;
    
private:
    unsigned char Lin2MuLaw(int16_t pcm_val);
    
    short MuLaw2Lin(uint8_t u_val);
    
    float mOutScale { 1.0f/32767.0f };
    
    int mSampleRate { 44100 };
    int mNumChannels { 2 };
    int mResamplingFilterOrder { 8 };
    std::vector<int> mDownsamplingCounter { 0, 0 };
    std::vector<float> mDownsamplingInput { 0.0f, 0.0f };
    
    LofiProcessorParameters parameters;
    
    using IIR = juce::dsp::IIR::Filter<float>;
    std::vector<std::vector<IIR>> preFilters;
    std::vector<std::vector<IIR>> postFilters;
    
    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> mFilterCoefficientsArray;
};

//==============================================================================
class GSMProcessor : public LofiProcessorBase
{
public:
    GSMProcessor();
    
    ~GSMProcessor() override;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
        
    void reset() override;
    
    LofiProcessorParameters& getParameters() override;
    
    void setParameters(const LofiProcessorParameters& params) override;
    
private:
    std::unique_ptr<gsm_state> mEncode = std::make_unique<gsm_state>();
    std::unique_ptr<gsm_state> mDecode = std::make_unique<gsm_state>();
    std::unique_ptr<gsm_signal[]> mGsmSignalInput = std::make_unique<gsm_signal[]>(160);
    std::unique_ptr<gsm_signal[]> mGsmSignal = std::make_unique<gsm_signal[]>(160);
    std::unique_ptr<gsm_signal[]> mGsmSignalOutput = std::make_unique<gsm_signal[]>(160);
    std::unique_ptr<gsm_byte[]> mGsmFrame = std::make_unique<gsm_byte[]>(33);
    
    LofiProcessorParameters parameters;
    int mSampleRate { 44100 };
    int mGsmSignalCounter { 0 };
    int mDownsamplingCounter { 0 };
    int mResamplingFilterOrder { 8 };
    
    float mCurrentSample { 0.0f };
    
    using IIR = juce::dsp::IIR::Filter<float>;
    IIR mLowCutFilter;
    std::vector<IIR> mPreFilters;
    std::vector<IIR> mPostFilters;
    
    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> mFilterCoefficientsArray;
};

//==============================================================================
class DownsampleAndFilter : public juce::dsp::ProcessorBase
{
public:
    DownsampleAndFilter() = default;
    
    ~DownsampleAndFilter() = default;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    
    void processBuffer(juce::AudioBuffer<float>& buffer);
    
    void reset() override;
    
    void setDownsampling(int newDownsampling);
    
private:
    int sampleRate { 44100 };
    int downsampling { 2 };
    int prevDownsampling { 0 };
    std::vector<int> downsamplingCounter { 0, 0 };
    float cutoff { 4410 };
    
    std::vector<float> currentSample { 0.0f, 0.0f };
    
    std::vector<Line<float>> resamplingRamps { Line<float>(), Line<float>() };
    
    using IIR = juce::dsp::IIR::Filter<float>;
    //IIR preFilter1L;
    //IIR preFilter1R;
    std::vector<juce::dsp::IIR::Filter<float>> preFilter1;
    std::vector<juce::dsp::IIR::Filter<float>> preFilter2;
    std::vector<juce::dsp::IIR::Filter<float>> preFilter3;
    std::vector<juce::dsp::IIR::Filter<float>> preFilter4;
    
    std::vector<juce::dsp::IIR::Filter<float>> postFilter1;
    std::vector<juce::dsp::IIR::Filter<float>> postFilter2;
    std::vector<juce::dsp::IIR::Filter<float>> postFilter3;
    std::vector<juce::dsp::IIR::Filter<float>> postFilter4;
    
    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> filterCoefficientsArray;
};

//==============================================================================
class ChebyDrive : public juce::dsp::ProcessorBase
{
public:
    ChebyDrive() = default;
    
    ~ChebyDrive() = default;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    
    void reset() override;
    
    void setDrive(float newDrive);
    
private:
    float drive { 0.0f };
};

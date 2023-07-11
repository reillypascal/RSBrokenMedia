/*
  ==============================================================================

    Make processor base class?

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Modulators.h"

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
class MuLaw : public juce::dsp::ProcessorBase
{
public:
    MuLaw() = default;
    
    ~MuLaw() = default;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    
    void reset() override;
    
private:
    unsigned char Lin2MuLaw(int16_t pcm_val);
    
    short MuLaw2Lin(uint8_t u_val);
    
    int sampleRate = 44100;
    
    float mOutScale = 1.0f/32767.0f;
};

//==============================================================================
class GSMProcessor : public juce::dsp::ProcessorBase
{
public:
    GSMProcessor() = default;
    
    ~GSMProcessor() = default;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    
    void processBuffer(juce::AudioBuffer<float>& buffer);
    
    void reset() override;
    
private:
    gsm encode = gsm_create();
    gsm decode = gsm_create();
    gsm_signal* gsmSignalInput = new gsm_signal[160];
    gsm_signal* gsmSignal = new gsm_signal[160];
    gsm_signal* gsmSignalOutput = new gsm_signal[160];
    gsm_frame gsmFrame;
    int gsmSignalCounter = 0;
    
    int sampleRate = 44100;
};

//==============================================================================
class DownsampleAndFilter : public juce::dsp::ProcessorBase
{
public:
    DownsampleAndFilter() = default;
    
    ~DownsampleAndFilter() = default;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    
    void reset() override;
    
    void setDownsampling(int newDownsampling);
    
private:
    int sampleRate = 44100;
    int downsampling = 2;
    float cutoff = 4410;
    
    std::vector<Line<float>> resamplingRamps { Line<float>(), Line<float>() };
    
    juce::dsp::IIR::Filter<float> preFilter1;
    juce::dsp::IIR::Filter<float> preFilter2;
    juce::dsp::IIR::Filter<float> preFilter3;
    juce::dsp::IIR::Filter<float> preFilter4;
    
    juce::dsp::IIR::Filter<float> postFilter1;
    juce::dsp::IIR::Filter<float> postFilter2;
    juce::dsp::IIR::Filter<float> postFilter3;
    juce::dsp::IIR::Filter<float> postFilter4;
    
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

/*
  ==============================================================================

 - Bitcrusher
 - Chebyshev Drive
 - MuLaw
 - GSM 06.10
 - Add saturation based on vb.mulaw~

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

class Bitcrusher : public LofiProcessorBase
{
public:
    Bitcrusher();
    
    ~Bitcrusher() override;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void reset() override;
    
    LofiProcessorParameters& getParameters() override;
    
    void setParameters(const LofiProcessorParameters& params) override;
    
private:
    LofiProcessorParameters parameters;
    
    int mSampleRate = 44100;
};

//==============================================================================
class ChebyDrive : public LofiProcessorBase
{
public:
    ChebyDrive();
    
    ~ChebyDrive() override;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void reset() override;
    
    LofiProcessorParameters& getParameters() override;
    
    void setParameters(const LofiProcessorParameters& params) override;
    
private:
    LofiProcessorParameters parameters;
};

//==============================================================================
class SaturationProcessor : public LofiProcessorBase
{
public:
    SaturationProcessor();
    
    ~SaturationProcessor() override;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void reset() override;
    
    LofiProcessorParameters& getParameters() override;
    
    void setParameters(const LofiProcessorParameters& params) override;
private:
    int mSampleRate { 44100 };
    int mNumChannels { 2 };
    
    LofiProcessorParameters parameters;
    
    float softClip(float x);
    
//    juce::dsp::IIR::Coefficients<float>::Ptr mLowCutFilterCoefficients;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> mLowCutFilter;
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
    
    const int cBias = 0x84;
    const int cClip = 32635;
    
    constexpr static char MuLawCompressTable[256]
    {
        0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
        4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
    };
    
    constexpr static short MuLawDecompressTable[256]
    {
         -32124,-31100,-30076,-29052,-28028,-27004,-25980,-24956,
         -23932,-22908,-21884,-20860,-19836,-18812,-17788,-16764,
         -15996,-15484,-14972,-14460,-13948,-13436,-12924,-12412,
         -11900,-11388,-10876,-10364, -9852, -9340, -8828, -8316,
          -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
          -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
          -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
          -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
          -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
          -1372, -1308, -1244, -1180, -1116, -1052,  -988,  -924,
           -876,  -844,  -812,  -780,  -748,  -716,  -684,  -652,
           -620,  -588,  -556,  -524,  -492,  -460,  -428,  -396,
           -372,  -356,  -340,  -324,  -308,  -292,  -276,  -260,
           -244,  -228,  -212,  -196,  -180,  -164,  -148,  -132,
           -120,  -112,  -104,   -96,   -88,   -80,   -72,   -64,
            -56,   -48,   -40,   -32,   -24,   -16,    -8,     -1,
          32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
          23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
          15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
          11900, 11388, 10876, 10364,  9852,  9340,  8828,  8316,
           7932,  7676,  7420,  7164,  6908,  6652,  6396,  6140,
           5884,  5628,  5372,  5116,  4860,  4604,  4348,  4092,
           3900,  3772,  3644,  3516,  3388,  3260,  3132,  3004,
           2876,  2748,  2620,  2492,  2364,  2236,  2108,  1980,
           1884,  1820,  1756,  1692,  1628,  1564,  1500,  1436,
           1372,  1308,  1244,  1180,  1116,  1052,   988,   924,
            876,   844,   812,   780,   748,   716,   684,   652,
            620,   588,   556,   524,   492,   460,   428,   396,
            372,   356,   340,   324,   308,   292,   276,   260,
            244,   228,   212,   196,   180,   164,   148,   132,
            120,   112,   104,    96,    88,    80,    72,    64,
             56,    48,    40,    32,    24,    16,     8,     0
    };
    
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
//class DownsampleAndFilter : public juce::dsp::ProcessorBase
//{
//public:
//    DownsampleAndFilter() = default;
//    
//    ~DownsampleAndFilter() = default;
//    
//    void prepare(const juce::dsp::ProcessSpec& spec) override;
//    
//    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
//    
//    void processBuffer(juce::AudioBuffer<float>& buffer);
//    
//    void reset() override;
//    
//    void setDownsampling(int newDownsampling);
//    
//private:
//    int sampleRate { 44100 };
//    int downsampling { 2 };
//    int prevDownsampling { 0 };
//    std::vector<int> downsamplingCounter { 0, 0 };
//    float cutoff { 4410 };
//    
//    std::vector<float> currentSample { 0.0f, 0.0f };
//    
//    std::vector<Line<float>> resamplingRamps { Line<float>(), Line<float>() };
//    
//    using IIR = juce::dsp::IIR::Filter<float>;
//    //IIR preFilter1L;
//    //IIR preFilter1R;
//    std::vector<juce::dsp::IIR::Filter<float>> preFilter1;
//    std::vector<juce::dsp::IIR::Filter<float>> preFilter2;
//    std::vector<juce::dsp::IIR::Filter<float>> preFilter3;
//    std::vector<juce::dsp::IIR::Filter<float>> preFilter4;
//    
//    std::vector<juce::dsp::IIR::Filter<float>> postFilter1;
//    std::vector<juce::dsp::IIR::Filter<float>> postFilter2;
//    std::vector<juce::dsp::IIR::Filter<float>> postFilter3;
//    std::vector<juce::dsp::IIR::Filter<float>> postFilter4;
//    
//    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> filterCoefficientsArray;
//};

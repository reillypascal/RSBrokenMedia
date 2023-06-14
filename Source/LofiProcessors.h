/*
  ==============================================================================

    Make processor base class?

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

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

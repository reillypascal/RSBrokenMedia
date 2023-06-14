/*
  ==============================================================================

    

  ==============================================================================
*/

#include "LofiProcessors.h"

void Bitcrusher::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
}

void Bitcrusher::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inBlock = context.getInputBlock();
    auto& outBlock = context.getOutputBlock();
    
    jassert(inBlock.getNumChannels() == outBlock.getNumChannels());
    jassert(inBlock.getNumSamples() == outBlock.getNumSamples());
    
    int len = static_cast<int>(inBlock.getNumSamples());
    int numChannels = static_cast<int>(inBlock.getNumChannels());
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* src = inBlock.getChannelPointer(channel);
        auto* dst = outBlock.getChannelPointer(channel);
        
        for (int sample = 0; sample < len; ++sample)
        {
            
            // reduce bit depth
            float totalQLevels = powf(2.0f, bitDepth);
            float val = src[sample];
            float remainder = fmodf(val, 1.0f/totalQLevels);
            
            // quantize
            dst[sample] = val - remainder;
            
            //dst[sample] = src[sample];
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

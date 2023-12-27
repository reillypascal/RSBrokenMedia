/*
  ==============================================================================

    Audio processor class interface

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BrokenPlayer.h"
#include "CircularBuffer.h"
#include "LofiProcessors.h"
#include "Utilities.h"

struct ProcessorFactory
{
    std::unique_ptr<LofiProcessorBase> create(int type)
    {
        auto iter = processorMapping.find(type);
        if (iter != processorMapping.end())
            return iter->second();
        
        return nullptr;
    }
    
    std::map<int,
             std::function<std::unique_ptr<LofiProcessorBase>()>> processorMapping
    {
        { 1, []() { return std::make_unique<MuLawProcessor>(); } },
        { 2, []() { return std::make_unique<GSMProcessor>(); } }
    };
};

//==============================================================================
/**
*/
class RSBrokenMediaAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    RSBrokenMediaAudioProcessor();
    ~RSBrokenMediaAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    void setUseDawClock(bool shouldUseDawClock);
private:
    juce::AudioProcessorValueTreeState parameters;
    
    juce::AudioPlayHead* audioPlayHead { nullptr };
    LockGuardedPosInfo lastPosInfo;
    bool useDawClock { false };
    float lastClock { -1 };
    
    ProcessorFactory processorFactory {};
    std::unique_ptr<LofiProcessorBase> slotProcessor = std::unique_ptr<LofiProcessorBase> {};
    LofiProcessorParameters processorParameters;
    
    BrokenPlayer brokenPlayer;
    juce::dsp::DryWetMixer<float> dryWetMixer;
    
    int slotCodec { 0 };
    int prevSlotCodec { 0 };
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RSBrokenMediaAudioProcessor)
};

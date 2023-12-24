/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

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
    
    std::atomic<float>* analogFXParameter = nullptr;
    std::atomic<float>* digitalFXParameter = nullptr;
    std::atomic<float>* lofiFXParameter = nullptr;
    
    std::atomic<float>* clockSpeedParameter = nullptr;
    juce::AudioParameterChoice* clockSpeedNoteParameter = nullptr; // updated to give nullptr
    bool useDawClock = false;
    std::atomic<float>* bufferLengthParameter = nullptr;
    std::atomic<float>* repeatsParameter = nullptr;
    std::atomic<float>* dryWetMixParameter = nullptr;
    
    juce::AudioParameterChoice* codecMenuParameter = nullptr;
    juce::AudioParameterChoice* downsamplingMenuParameter = nullptr;
    
    juce::AudioPlayHead* audioPlayHead = nullptr;
    LockGuardedPosInfo lastPosInfo;
    float lastClock = -1;
    
    BrokenPlayer brokenPlayer;
    juce::dsp::DryWetMixer<float> dryWetMixer;
    
    ProcessorFactory processorFactory {};
    
    std::unique_ptr<LofiProcessorBase> slotProcessor = std::unique_ptr<LofiProcessorBase> {};
    
    LofiProcessorParameters processorParameters;
    
    int slotCodec { 0 };
    int prevSlotCodec { 0 };
    
//    DownsampleAndFilter downsampleAndFilter;
    GSMProcessor gsmProcessor;
    MuLawProcessor muLawProcessor;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RSBrokenMediaAudioProcessor)
};

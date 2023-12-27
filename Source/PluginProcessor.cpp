/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RSBrokenMediaAudioProcessor::RSBrokenMediaAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    parameters(*this, nullptr, juce::Identifier("RSBrokenMedia"), {
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "analogFX", 1 },
                                                    "Analog FX",
                                                    0.0f,
                                                    1.0f,
                                                    0.35f),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "digitalFX", 1 },
                                                    "Digital FX",
                                                    0.0f,
                                                    1.0f,
                                                    0.35f),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "lofiFX", 1 },
                                                    "Distortion FX",
                                                    0.0f,
                                                    1.0f,
                                                    0.0f),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "clockSpeed", 1 },
                                                    "Clock Speed",
                                                    juce::NormalisableRange<float>(80,
                                                                                   2000,
                                                                                   5),
                                                    1000),
        std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "clockSpeedNote", 1 },
                                                    "Clock Speed (Note)",
                                                    juce::StringArray { "4 bars", "2 bars", "1 bar", "1/2d", "1/2", "1/4d", "1/4", "1/8d", "1/8", "1/16" },
                                                    4),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "bufferLength", 1 },
                                                    "Buffer Length",
                                                    juce::NormalisableRange<float>(12,
                                                                                   8000,
                                                                                   1),
                                                    1500),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "repeats", 1 },
                                                    "Repeats",
                                                    juce::NormalisableRange<float>(1,
                                                                                   64,
                                                                                   1),
                                                    1),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "dryWetMix", 1 },
                                                    "Dry/Wet Mix",
                                                    0.0f,
                                                    1.0f,
                                                    0.4f),
        std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "clockMode", 1 },
                                                    "Clock Mode",
                                                    false),
        std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "distType", 1 },
                                                    "Dist Menu",
                                                     juce::StringArray { "Bitcrush", "Saturation" },
                                                    0),
        std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "codec", 1 },
                                                    "Codec Menu",
                                                     juce::StringArray { "None", "MuLaw", "GSM" },
                                                    0),
        std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "downsampling", 1 },
                                                    "Downsampling Menu",
                                                     juce::StringArray { "None", "x2", "x3", "x4", "x5", "x6", "x7", "x8" },
                                                    0)
})
{
}

RSBrokenMediaAudioProcessor::~RSBrokenMediaAudioProcessor()
{
}

//==============================================================================
const juce::String RSBrokenMediaAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RSBrokenMediaAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RSBrokenMediaAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RSBrokenMediaAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RSBrokenMediaAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RSBrokenMediaAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RSBrokenMediaAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RSBrokenMediaAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RSBrokenMediaAudioProcessor::getProgramName (int index)
{
    return {};
}

void RSBrokenMediaAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RSBrokenMediaAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::AudioChannelSet layout;
    for (int channel = 0; channel < getTotalNumInputChannels(); ++channel)
    {
        layout.addChannel(static_cast<juce::AudioChannelSet::ChannelType>(channel + 1));
    }
    brokenPlayer.setChannelLayoutOfBus(true, 0, layout);
    brokenPlayer.prepareToPlay(sampleRate, samplesPerBlock);
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    
    dryWetMixer.prepare(spec);
}

void RSBrokenMediaAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RSBrokenMediaAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void RSBrokenMediaAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    //======== buffer safety ========
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    //======== tempo ========
    audioPlayHead = this->getPlayHead();
    lastPosInfo.set(audioPlayHead->getPosition().orFallback(juce::AudioPlayHead::PositionInfo {}));
    double quarterNotes = lastPosInfo.get().getPpqPosition().orFallback(0.0);
    std::vector<float> clockNoteValues { 16.0f, 8.0f, 4.0f, 3.0f, 2.0f, 1.5f, 1.0f, 0.75f, 0.5f, 0.25f };
    
    //======== get parameters ========
    float analogFX = parameters.getRawParameterValue("analogFX")->load();
    
    float digitalFX = parameters.getRawParameterValue("digitalFX")->load();
    
    float lofiFX = parameters.getRawParameterValue("lofiFX")->load();
    
    float clockSpeed = parameters.getRawParameterValue("clockSpeed")->load() * (getSampleRate() / 1000);
    
    int clockSpeedNoteIndex = static_cast<juce::AudioParameterChoice*>(parameters.getParameter("clockSpeedNote"))->getIndex();
    
    int bufferLength = static_cast<int>(parameters.getRawParameterValue("bufferLength")->load() * 44.1f); // check menu data type
    
    int numRepeats = static_cast<int>(parameters.getRawParameterValue("repeats")->load()); // check menu data type
    
    float dryWetMix = parameters.getRawParameterValue("dryWetMix")->load();
    
    // ======== mix in dry ========
    dryWetMixer.setWetMixProportion(dryWetMix);
    dryWetMixer.pushDrySamples(juce::dsp::AudioBlock<float> {buffer});
    
    //======== constant codec processing ========
    slotCodec = static_cast<juce::AudioParameterChoice*>(parameters.getParameter("codec"))->getIndex();
    
    if (slotCodec != prevSlotCodec)
    {
        slotProcessor = processorFactory.create(slotCodec);
        
        if (slotProcessor != nullptr)
        {
            juce::dsp::ProcessSpec spec;
            spec.sampleRate = getSampleRate();
            spec.maximumBlockSize = buffer.getNumSamples();
            spec.numChannels = buffer.getNumChannels();
            
            slotProcessor->prepare(spec);
        }
        
        prevSlotCodec = slotCodec;
    }
    
    if (slotProcessor != nullptr)
    {
        processorParameters = slotProcessor->getParameters();
        
        processorParameters.downsampling = static_cast<juce::AudioParameterChoice*>(parameters.getParameter("downsampling"))->getIndex() + 1;
        
        slotProcessor->setParameters(processorParameters);
        
        slotProcessor->processBlock(buffer, midiMessages);
    }
    
    //======== broken player ========
    brokenPlayer.setAnalogFX(analogFX);
    brokenPlayer.setDigitalFX(digitalFX);
    brokenPlayer.setLofiFX(lofiFX);
    
    brokenPlayer.setDistortionType(static_cast<juce::AudioParameterChoice*>(parameters.getParameter("distType"))->getIndex());
    
    brokenPlayer.useExternalClock(useDawClock);
    if (useDawClock == true)
    {
        float currentClock = floor(quarterNotes / clockNoteValues.at(clockSpeedNoteIndex));
        if (lastClock != currentClock)
        {
            brokenPlayer.receiveClockedPulse();
            lastClock = currentClock;
        }
    }
    else
        brokenPlayer.setClockSpeed(clockSpeed);
    
    brokenPlayer.setBufferLength(bufferLength);
    brokenPlayer.newNumRepeats(numRepeats);
    brokenPlayer.processBlock(buffer, midiMessages);
    
    //======== mix in wet ========
    dryWetMixer.mixWetSamples(juce::dsp::AudioBlock<float> {buffer});
}

//==============================================================================
bool RSBrokenMediaAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RSBrokenMediaAudioProcessor::createEditor()
{
    return new RSBrokenMediaAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void RSBrokenMediaAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary(*xml, destData);
}

void RSBrokenMediaAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RSBrokenMediaAudioProcessor();
}

//==============================================================================
void RSBrokenMediaAudioProcessor::setUseDawClock(bool shouldUseDawClock)
{
    useDawClock = shouldUseDawClock;
    auto internalClockParameter = parameters.getParameter("clockSpeed");
    internalClockParameter->setValueNotifyingHost(internalClockParameter->getDefaultValue());
    auto externalClockParameter = parameters.getParameter("clockSpeedNote");
    externalClockParameter->setValueNotifyingHost(externalClockParameter->getDefaultValue());
}

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
                                                    675),
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
                                                    8),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "dryWetMix", 1 },
                                                    "Dry/Wet Mix",
                                                    0.0f,
                                                    1.0f,
                                                    0.4f),
        std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "codec", 1 },
                                                    "Codec Menu",
                                                     juce::StringArray { "None", "MuLaw", "GSM" },
                                                    0),
        std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "downsampling", 1 },
                                                    "Downsampling Menu",
                                                     juce::StringArray { "None", "x2", "x4", "x8", "x16" },
                                                    0)
})
{
    analogFXParameter = parameters.getRawParameterValue("analogFX");
    digitalFXParameter = parameters.getRawParameterValue("digitalFX");
    lofiFXParameter = parameters.getRawParameterValue("lofiFX");
    
    clockSpeedParameter = parameters.getRawParameterValue("clockSpeed");
    bufferLengthParameter = parameters.getRawParameterValue("bufferLength");
    repeatsParameter = parameters.getRawParameterValue("repeats");
    dryWetMixParameter = parameters.getRawParameterValue("dryWetMix");
    
    codecMenuParameter = static_cast<juce::AudioParameterChoice*>(parameters.getParameter("codec"));
    downsamplingMenuParameter = static_cast<juce::AudioParameterChoice*>(parameters.getParameter("downsampling"));
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
    
    muLaw.prepare(spec);
    downsampleAndFilter.prepare(spec);
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
    
    //======== get parameters ========
    float analogFX = static_cast<float>(*analogFXParameter);
    float digitalFX = static_cast<float>(*digitalFXParameter);
    float lofiFX = static_cast<float>(*lofiFXParameter);
    
    float clockSpeed = static_cast<float>(*clockSpeedParameter);
    int bufferLength = static_cast<int>(*bufferLengthParameter * 44.1f);
    //int numRepeats = static_cast<int>(*repeatsParameter);
    float dryWetMix = static_cast<float>(*dryWetMixParameter);
    
    int downsampling = static_cast<int>(*downsamplingMenuParameter);
    int codec = static_cast<int>(*codecMenuParameter);
    
    // ======== mix in dry ========
    dryWetMixer.setWetMixProportion(dryWetMix);
    dryWetMixer.pushDrySamples(juce::dsp::AudioBlock<float> {buffer});
    
    //======== constant downsampling ========
    juce::dsp::AudioBlock<float> preBrokenBlock { buffer };
    if (downsampling != 0)
    {
        if (downsampling == 1)
            downsampleAndFilter.setDownsampling(2);
        else if (downsampling == 2)
            downsampleAndFilter.setDownsampling(4);
        else if (downsampling == 3)
            downsampleAndFilter.setDownsampling(8);
        else if (downsampling == 4)
            downsampleAndFilter.setDownsampling(16);
        downsampleAndFilter.process(juce::dsp::ProcessContextReplacing<float>(preBrokenBlock));
    }
    
    //======== broken player ========
    brokenPlayer.setAnalogFX(analogFX);
    brokenPlayer.setDigitalFX(digitalFX);
    brokenPlayer.setLofiFX(lofiFX);
    
    brokenPlayer.setClockSpeed(clockSpeed);
    brokenPlayer.setBufferLength(bufferLength);
    
    brokenPlayer.processBlock(buffer, midiMessages);
    
    //======== constant codec processing ========
    juce::dsp::AudioBlock<float> postBrokenBlock { buffer };
    if (codec == 1) // if menu doesn't include GSM, need 2 to work
        muLaw.process(juce::dsp::ProcessContextReplacing<float>(postBrokenBlock));
    else if (codec == 2)
    {
        // gsm processing
    }
    
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

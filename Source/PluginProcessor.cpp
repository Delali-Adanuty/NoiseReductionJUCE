#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
}

NewProjectAudioProcessor::~NewProjectAudioProcessor() {}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const { return JucePlugin_Name; }
bool NewProjectAudioProcessor::acceptsMidi() const { return false; }
bool NewProjectAudioProcessor::producesMidi() const { return false; }
bool NewProjectAudioProcessor::isMidiEffect() const { return false; }
double NewProjectAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int NewProjectAudioProcessor::getNumPrograms() { return 1; }
int NewProjectAudioProcessor::getCurrentProgram() { return 0; }
void NewProjectAudioProcessor::setCurrentProgram(int) {}
const juce::String NewProjectAudioProcessor::getProgramName(int) { return {}; }
void NewProjectAudioProcessor::changeProgramName(int, const juce::String&) {}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);

    noiseReducer.init();

    std::fill(std::begin(inputFifo), std::end(inputFifo), 0.0f);
    std::fill(std::begin(outputFifo), std::end(outputFifo), 0.0f);

    fifoWriteIndex = 0;
    fifoReadIndex = 0;
    samplesInOutput = 0;
    hopCounter = 0;
}

void NewProjectAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    return true;
#endif
}
#endif

//==============================================================================
void NewProjectAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = getTotalNumOutputChannels();

    noiseReducer.setAlpha(alphaParameter.load());
    noiseReducer.setBeta(betaParameter.load());

    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = (numChannels > 1) ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        inputFifo[fifoWriteIndex] = leftChannel[i];
        fifoWriteIndex = (fifoWriteIndex + 1) % FFT_SIZE;

        float out = 0.0f;
        if (samplesInOutput > 0)
        {
            out = outputFifo[fifoReadIndex];
            fifoReadIndex = (fifoReadIndex + 1) % FFT_SIZE;
            samplesInOutput--;
        }

        leftChannel[i] = out;
        if (rightChannel != nullptr)
            rightChannel[i] = out;

        hopCounter++;
        if (hopCounter >= HOP_SIZE)
        {
            hopCounter = 0;

            std::array<float, FFT_SIZE> frameIn;
            std::array<float, HOP_SIZE> frameOut;
            frameIn.fill(0.0f);
            frameOut.fill(0.0f);

            for (int k = 0; k < FFT_SIZE; ++k)
            {
                int idx = (fifoWriteIndex + k) % FFT_SIZE;
                frameIn[k] = inputFifo[idx];
            }

            noiseReducer.processFrame(frameIn.data(), frameOut.data());

            int writePos = (fifoReadIndex + samplesInOutput) % FFT_SIZE;
            for (int k = 0; k < HOP_SIZE; ++k)
            {
                outputFifo[writePos] = frameOut[k];
                writePos = (writePos + 1) % FFT_SIZE;
            }
            samplesInOutput += HOP_SIZE;
        }
    }

    for (int ch = getTotalNumInputChannels(); ch < numChannels; ++ch)
        buffer.clear(ch, 0, numSamples);
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor(*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
}

void NewProjectAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
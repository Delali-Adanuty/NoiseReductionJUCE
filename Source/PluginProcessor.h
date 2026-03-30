#pragma once
#include <JuceHeader.h>
#include "NoiseReducer.h"

class NewProjectAudioProcessor : public juce::AudioProcessor
{
public:
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // alpha=2.0 is a safe starting point
    std::atomic<float> alphaParameter{ 2.0f };
    std::atomic<float> betaParameter{ 0.01f };

private:
    static constexpr int FFT_SIZE = 512;
    static constexpr int HOP_SIZE = 256;

    NoiseReducer noiseReducer;

    float inputFifo[FFT_SIZE] = {};
    float outputFifo[FFT_SIZE] = {};

    int  fifoWriteIndex = 0;
    int  fifoReadIndex = 0;
    int  samplesInOutput = 0;
    int  hopCounter = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessor)
};
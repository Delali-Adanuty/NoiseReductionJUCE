#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class NewProjectAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Timer
{
public:
    NewProjectAudioProcessorEditor(NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Timer fires every 100ms to update the button label while recording
    void timerCallback() override;

private:
    NewProjectAudioProcessor& audioProcessor;

    juce::Slider alphaSlider;
    juce::Label  alphaLabel;
    juce::Slider betaSlider;
    juce::Label  betaLabel;

    juce::TextButton recordButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessorEditor)
};
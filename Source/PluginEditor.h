/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class NewProjectAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    NewProjectAudioProcessorEditor(NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    // These two functions are called by JUCE to draw the window
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // A reference to the audio engine so we can send it new values
    NewProjectAudioProcessor& audioProcessor;

    // The visual components
    juce::Slider alphaSlider;
    juce::Label alphaLabel;

    juce::Slider betaSlider;
    juce::Label betaLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessorEditor)
};
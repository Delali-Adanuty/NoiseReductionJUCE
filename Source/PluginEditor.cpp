/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// 1. THE CONSTRUCTOR 
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor(NewProjectAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Set window size
    setSize(400, 300);

    // Configure the rotary slider
    alphaSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    alphaSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    alphaSlider.setRange(0.0, 9.0, 0.1);

    // Initial value from processor
    alphaSlider.setValue(audioProcessor.alphaParameter.load());

    // Link slider to the processor's atomic variable
    alphaSlider.onValueChange = [this]
        {
            audioProcessor.alphaParameter.store((float)alphaSlider.getValue());
        };

    // Label setup
    alphaLabel.setText("Subtraction Strength (Alpha)", juce::dontSendNotification);
    alphaLabel.setJustificationType(juce::Justification::centred);
    alphaLabel.attachToComponent(&alphaSlider, false);

    // Make visible
    addAndMakeVisible(alphaSlider);
    addAndMakeVisible(alphaLabel);

    betaSlider.setRange(0.01, 0.5, 0.01); 
    betaSlider.setValue(0.01);
    betaSlider.onValueChange = [this] {
        audioProcessor.betaParameter.store((float)betaSlider.getValue());
        };

    betaLabel.setText("Spectral Floor (Beta)", juce::dontSendNotification);
    betaLabel.setJustificationType(juce::Justification::centred);
    betaLabel.attachToComponent(&betaSlider, false);

    addAndMakeVisible(betaLabel);
    addAndMakeVisible(betaSlider);
 
}

// 2. THE DESTRUCTOR
NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
}

// 3. THE PAINT FUNCTION
void NewProjectAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background color
    g.fillAll(juce::Colour::fromRGB(40, 40, 40));
}

// 4. THE RESIZED FUNCTION
void NewProjectAudioProcessorEditor::resized()
{
    // Center the slider
    alphaSlider.setBounds(100, 50, 200, 200);

    alphaSlider.setBounds(50, 50, 150, 150);
    betaSlider.setBounds(200, 50, 150, 150);
}
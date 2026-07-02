#include "PluginProcessor.h"
#include "PluginEditor.h"

NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor(NewProjectAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(400, 340);

    // ── Alpha slider ─────────────────────────────────────────────────────────
    alphaSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    alphaSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    alphaSlider.setRange(0.0, 9.0, 0.1);
    alphaSlider.setValue(audioProcessor.alphaParameter.load());
    alphaSlider.onValueChange = [this]
        {
            audioProcessor.alphaParameter.store((float)alphaSlider.getValue());
        };
    alphaLabel.setText("Subtraction Strength (Alpha)", juce::dontSendNotification);
    alphaLabel.setJustificationType(juce::Justification::centred);
    alphaLabel.attachToComponent(&alphaSlider, false);
    addAndMakeVisible(alphaSlider);
    addAndMakeVisible(alphaLabel);

    // ── Beta slider ──────────────────────────────────────────────────────────
    betaSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    betaSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    betaSlider.setRange(0.01, 0.5, 0.01);
    betaSlider.setValue(audioProcessor.betaParameter.load());
    betaSlider.onValueChange = [this]
        {
            audioProcessor.betaParameter.store((float)betaSlider.getValue());
        };
    betaLabel.setText("Spectral Floor (Beta)", juce::dontSendNotification);
    betaLabel.setJustificationType(juce::Justification::centred);
    betaLabel.attachToComponent(&betaSlider, false);
    addAndMakeVisible(betaLabel);
    addAndMakeVisible(betaSlider);

    // ── Record button ────────────────────────────────────────────────────────
    recordButton.setButtonText("Start Recording");
    recordButton.onClick = [this]
        {
            if (audioProcessor.isCurrentlyRecording())
            {
                audioProcessor.stopRecording();
                recordButton.setButtonText("Start Recording");
                recordButton.setColour(juce::TextButton::buttonColourId,
                    juce::Colours::darkgrey);
                stopTimer();
            }
            else
            {
                audioProcessor.startRecording();
                recordButton.setButtonText("Stop Recording");
                recordButton.setColour(juce::TextButton::buttonColourId,
                    juce::Colours::red);
                // Poll every 100ms to keep button state in sync
                startTimer(100);
            }
        };
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    addAndMakeVisible(recordButton);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
    stopTimer();
}

void NewProjectAudioProcessorEditor::timerCallback()
{
    // If recording stopped externally (e.g. plugin reloaded), sync the button
    if (!audioProcessor.isCurrentlyRecording())
    {
        recordButton.setButtonText("Start Recording");
        recordButton.setColour(juce::TextButton::buttonColourId,
            juce::Colours::darkgrey);
        stopTimer();
    }
}

void NewProjectAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(40, 40, 40));
}

void NewProjectAudioProcessorEditor::resized()
{
    alphaSlider.setBounds(50, 50, 150, 150);
    betaSlider.setBounds(200, 50, 150, 150);
    recordButton.setBounds(100, 270, 200, 40);
}
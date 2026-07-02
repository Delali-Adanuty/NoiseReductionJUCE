#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <fstream>

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
    // Register WAV format so the format manager knows how to create WAV writers
    formatManager.registerBasicFormats();
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
    // Make sure we stop recording cleanly if the plugin is closed mid-recording
    stopRecording();
}

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
    juce::ignoreUnused(samplesPerBlock);

    currentSampleRate = sampleRate;
    noiseReducer.init();

    std::fill(std::begin(inputFifo), std::end(inputFifo), 0.0f);
    std::fill(std::begin(outputFifo), std::end(outputFifo), 0.0f);

    fifoWriteIndex = 0;
    fifoReadIndex = 0;
    samplesInOutput = 0;
    hopCounter = 0;
}

void NewProjectAudioProcessor::releaseResources()
{
    stopRecording();
}

void NewProjectAudioProcessor::startRecording()
{
    juce::File inputFile("C:/EXTRAS/Projects/JUCENoiseReduction/NewProject/resources/input_raw.wav");
    juce::File outputFile("C:/EXTRAS/Projects/JUCENoiseReduction/NewProject/resources/output_processed.wav");

    // ── TEMP DIAGNOSTIC ──────────────────────────────────────────────────────
    std::ofstream log("C:/EXTRAS/Projects/JUCENoiseReduction/NewProject/resources/recording_debug.txt");
    log << "startRecording called\n";
    log << "Input path: " << inputFile.getFullPathName().toStdString() << "\n";
    log << "Output path: " << outputFile.getFullPathName().toStdString() << "\n";
    log << "Can write to C:/: " << (int)inputFile.getParentDirectory().hasWriteAccess() << "\n";
    log.close();
    // ─────────────────────────────────────────────────────────────────────────

    inputFile.deleteFile();
    outputFile.deleteFile();

    auto* wavFormat = formatManager.findFormatForFileExtension("wav");
    if (wavFormat == nullptr)
    {
        std::ofstream log("C:/EXTRAS/Projects/JUCENoiseReduction/recording_debug.txt");
        log << "FAILED: wavFormat is null\n"; return;
    }

    auto inputStream = std::make_unique<juce::FileOutputStream>(inputFile);
    auto outputStream = std::make_unique<juce::FileOutputStream>(outputFile);

    if (inputStream->failedToOpen() || outputStream->failedToOpen())
    {
        std::ofstream l("C:/Users/delal/Desktop/recording_debug.txt", std::ios::app);
        l << "FAILED: streams failed to open\n";
        l << "inputStream failed: " << (int)inputStream->failedToOpen() << "\n";
        l << "outputStream failed: " << (int)outputStream->failedToOpen() << "\n";
        return;
    }

    auto* rawInputWriter = wavFormat->createWriterFor(
        inputStream.release(), currentSampleRate, 1, 16, {}, 0);

    auto* rawOutputWriter = wavFormat->createWriterFor(
        outputStream.release(), currentSampleRate, 1, 16, {}, 0);

    if (rawInputWriter == nullptr || rawOutputWriter == nullptr)
    {
        std::ofstream l("C:/recording_debug.txt", std::ios::app);
        l << "FAILED: createWriterFor returned null\n";
        l << "rawInputWriter null: " << (int)(rawInputWriter == nullptr) << "\n";
        l << "rawOutputWriter null: " << (int)(rawOutputWriter == nullptr) << "\n";
        return;
    }

    recordingThread.startThread();

    inputWriter = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(
        rawInputWriter, recordingThread, 32768);
    outputWriter = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(
        rawOutputWriter, recordingThread, 32768);

    isRecording.store(true);

    std::ofstream l("C:/recording_debug.txt", std::ios::app);
    l << "SUCCESS: recording started\n";
}

void NewProjectAudioProcessor::stopRecording()
{
    isRecording.store(false);

    // Destroy writers first — flushes buffers and closes files
    inputWriter.reset();
    outputWriter.reset();

    // Then stop the thread
    recordingThread.stopThread(1000);
}

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

    // Capture raw input BEFORE we overwrite the buffer with processed output.
    // We write one channel (mono) since our noise reducer is mono.
    if (isRecording.load() && inputWriter != nullptr)
    {
        // AudioFormatWriter::ThreadedWriter::write expects a float** (array of channel pointers)
        // and a number of samples. We pass the address of leftChannel to get float**
        const float* inputChannels[1] = { leftChannel };
        inputWriter->write(inputChannels, numSamples);
    }

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
                frameIn[k] = inputFifo[(fifoWriteIndex + k) % FFT_SIZE];

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

    // Write processed output AFTER the buffer has been filled with processed samples
    if (isRecording.load() && outputWriter != nullptr)
    {
        const float* outputChannels[1] = { leftChannel };
        outputWriter->write(outputChannels, numSamples);
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

void NewProjectAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
}

void NewProjectAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
/*
  ==============================================================================

    NoiseReducer.cpp
    Created: 29 Mar 2026 1:49:15am
    Author:  Van-Dyck Adanuty

  ==============================================================================
*/

#include "NoiseReducer.h"
#include <cmath>
#include <complex>
#include <algorithm>
#include <fstream>

NoiseReducer::NoiseReducer(int /*fftOrder*/)
    : alpha(2.0f),
    beta(0.01f),
    fft(FFT_ORDER),
    hannWindow(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann),
    noiseEnergy(0.01f),
    vadThreshold(2.0f)
{
    timeFrame.fill(0.0f);
    complexBuffer.fill(0.0f);
    fftOutput.fill(0.0f);
    noiseMagnitude.fill(0.01f);
    overlapAccum.fill(0.0f);
}

void NoiseReducer::init()
{
    timeFrame.fill(0.0f);
    complexBuffer.fill(0.0f);
    fftOutput.fill(0.0f);
    noiseMagnitude.fill(0.01f);
    overlapAccum.fill(0.0f);
    noiseEnergy = 0.01f;
    frameCount = 0;
}

void NoiseReducer::processFrame(const float* input, float* output)
{
    // --- 1. Window ---
    for (int i = 0; i < FFT_SIZE; ++i)
        timeFrame[i] = input[i];

    hannWindow.multiplyWithWindowingTable(timeFrame.data(), FFT_SIZE);

    // --- 2. Pack ---
    for (int i = 0; i < FFT_SIZE; ++i)
    {
        complexBuffer[i * 2] = timeFrame[i];
        complexBuffer[i * 2 + 1] = 0.0f;
    }

    auto* cx = reinterpret_cast<std::complex<float>*>(complexBuffer.data());
    auto* cxOut = reinterpret_cast<std::complex<float>*>(fftOutput.data());

    // --- 3. Forward FFT ---
    fft.perform(cx, cxOut, false);

    // --- 4. Voice Activity Detection ---
    float frameEnergy = 0.0f;
    for (int i = 1; i < NUM_BINS; ++i)
    {
        const float re = fftOutput[i * 2];
        const float im = fftOutput[i * 2 + 1];
        frameEnergy += re * re + im * im;
    }

    bool speechDetected = false;
    const float digitalSilenceThreshold = 0.0001f; 

    if (frameEnergy < digitalSilenceThreshold)
    {
        speechDetected = false;
    }
    else if (frameCount < 20)
    {
        frameCount++;
        speechDetected = false;

        if (frameCount == 1)
            noiseEnergy = frameEnergy;
        else
            noiseEnergy = 0.5f * noiseEnergy + 0.5f * frameEnergy;
    }
    else
    {
       
        speechDetected = (frameEnergy > vadThreshold * noiseEnergy);
        if (!speechDetected)
            noiseEnergy = 0.95f * noiseEnergy + 0.05f * frameEnergy;
    }

    // --- 5. Spectral Subtraction ---
    fftOutput[1] = 0.0f;
    for (int i = 1; i < NUM_BINS; ++i)
    {
        const float re = fftOutput[i * 2];
        const float im = fftOutput[i * 2 + 1];
        const float mag = std::sqrt(re * re + im * im) + 1e-10f;
        const float phase = std::atan2(im, re);

        if (!speechDetected && frameEnergy >= digitalSilenceThreshold)
        {
            if (frameCount <= 20)
                noiseMagnitude[i] = (frameCount == 1) ? mag : (0.5f * noiseMagnitude[i] + 0.5f * mag);
            else if (mag > noiseMagnitude[i])
                noiseMagnitude[i] = 0.90f * noiseMagnitude[i] + 0.10f * mag;
            else
                noiseMagnitude[i] = 0.995f * noiseMagnitude[i] + 0.005f * mag;
        }

        if (!std::isfinite(noiseMagnitude[i]))
            noiseMagnitude[i] = 0.01f;

        float clean_mag = mag - alpha * noiseMagnitude[i];
        clean_mag = std::max(clean_mag, beta * mag);

        if (!std::isfinite(clean_mag))
            clean_mag = 0.0f;

        fftOutput[i * 2] = clean_mag * std::cos(phase);
        fftOutput[i * 2 + 1] = clean_mag * std::sin(phase);

        const int mirror = FFT_SIZE - i;
        fftOutput[mirror * 2] = fftOutput[i * 2];
        fftOutput[mirror * 2 + 1] = -fftOutput[i * 2 + 1];
    }

    // --- 6. Inverse FFT ---
    fft.perform(cxOut, cx, true);

    // --- 7. Output the centre HOP_SIZE samples ---
    for (int i = 0; i < FFT_SIZE; ++i)
        overlapAccum[i] += complexBuffer[i * 2];

    for (int i = 0; i < HOP_SIZE; ++i)
        output[i] = overlapAccum[i] * 0.5f;

    for (int i = 0; i < FFT_SIZE - HOP_SIZE; ++i)
        overlapAccum[i] = overlapAccum[i + HOP_SIZE];


    for (int i = FFT_SIZE - HOP_SIZE; i < FFT_SIZE; ++i)
        overlapAccum[i] = 0.0f;
}
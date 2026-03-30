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
    hannWindow(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann)
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

    // --- 4. Spectral subtraction ---
    fftOutput[1] = 0.0f;

    for (int i = 1; i < NUM_BINS; ++i)
    {
        const float re = fftOutput[i * 2];
        const float im = fftOutput[i * 2 + 1];
        const float mag = std::sqrt(re * re + im * im) + 1e-10f;
        const float phase = std::atan2(im, re);

        if (mag > noiseMagnitude[i])
            noiseMagnitude[i] = 0.90f * noiseMagnitude[i] + 0.10f * mag;
        else
            noiseMagnitude[i] = 0.995f * noiseMagnitude[i] + 0.005f * mag;

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

    // --- 5. Inverse FFT ---
    fft.perform(cxOut, cx, true);

    // --- 6. Output the centre HOP_SIZE samples ---
    const int start = FFT_SIZE / 4;  // 128
    for (int i = 0; i < HOP_SIZE; ++i)
        output[i] = complexBuffer[(start + i) * 2];
}
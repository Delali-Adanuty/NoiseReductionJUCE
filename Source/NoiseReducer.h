/*
  ==============================================================================

    NoiseReducer.h
    Created: 29 Mar 2026 1:49:15am
    Author:  Van-Dyck Adanuty

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <complex>
#include <array>

class NoiseReducer
{
public:
    NoiseReducer(int fftOrder = 9);
    ~NoiseReducer() = default;

    void init();
    void processFrame(const float* input, float* output);

    void setAlpha(float a) { alpha = a; }
    void setBeta(float b) { beta = b; }

private:
    static constexpr int FFT_ORDER = 9;
    static constexpr int FFT_SIZE = 1 << FFT_ORDER;  // 512
    static constexpr int HOP_SIZE = FFT_SIZE / 2;     // 256
    static constexpr int NUM_BINS = FFT_SIZE / 2;     // 256

    float alpha;
    float beta;

    juce::dsp::FFT                      fft;
    juce::dsp::WindowingFunction<float> hannWindow;

    std::array<float, FFT_SIZE>         timeFrame;
    std::array<float, FFT_SIZE * 2>     complexBuffer;  // interleaved re/im, FFT input
    std::array<float, FFT_SIZE * 2>     fftOutput;      // separate FFT output buffer
    std::array<float, NUM_BINS>         noiseMagnitude;
    std::array<float, FFT_SIZE>         overlapAccum;
};
# Real-Time Noise Reducer (Standalone App & Plugin)

A real-time C++ audio processor built with the JUCE framework that utilizes smart spectral subtraction to intelligently remove background noise and room hiss from live microphone input.

## Overview

This processor operates either as a **Standalone Desktop App** (utilizing your native microphone and audio drivers) or as a traditional VST3/AU plugin within a Digital Audio Workstation (DAW).

Unlike a traditional noise gate that abruptly mutes audio below a certain threshold, this engine performs continuous time-frequency analysis. It profiles ambient room noise and surgically subtracts it from the incoming frequency spectrum while actively protecting the harmonic components of the primary voice.

---

## Performance Analysis

The DSP pipeline has been mathematically verified to deliver significant attenuation of broadband interference (such as HVAC systems, computer fans, and ambient room noise) without destroying the vocal signal.

Below is the verified performance reduction against standard broadband fan noise:

![Fan Noise Performance](/docs/images/fan_noise_db_reduction.png)

> **Figure 1:** _Frequency response analysis demonstrating ~15 dB of consistent noise attenuation. Note the targeted dip in reduction between 0–3 kHz; this is the algorithm's protection zone, allowing fundamental human speech frequencies to pass through with minimal processing._

![Fan Noise Spectrogram](/docs/images/fan_noise_spectrogram.png)

> **Figure 2:** _Time-frequency visualization comparing raw microphone input (top) to the processed output (bottom). The continuous grey noise floor is suppressed while transient vocal structures are preserved._

---

## Architecture & Features

- **Real-Time FFT Pipeline:** Utilizes `juce::dsp::FFT` for high-performance, low-latency frequency domain transformation.
- **Smart Voice Activity Detection (VAD):** Features an initialization threshold that prevents the algorithm from locking onto digital silence. It accurately isolates speech to freeze the noise profile dynamically.
- **Asymmetric Noise Tracking:** Intelligently adapts to changing environments. The algorithm learns quickly during ambient silence to track background hiss, but updates its profile exceptionally slowly when speech is detected to prevent vocal cancellation.
- **Overlap-Add (OLA) Reconstruction:** Leverages a 25% hop-size with a Hann windowed analysis/synthesis phase to ensure seamless, artifact-free audio reconstruction.

---

## Usage & Tuning Parameters

The spectral subtractor can be finely tuned to handle different environmental noise profiles using three primary parameters:

| Parameter                   | Recommended Range | Description                                                                                                                                                                |
| :-------------------------- | :---------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Alpha (Oversubtraction)** | `1.0` – `5.0`     | Determines the aggression of the subtraction. Higher values remove more noise but can introduce a "hollow" or "thin" vocal quality.                                        |
| **Beta (Spectral Floor)**   | `0.01` – `0.20`   | Sets a minimum noise floor limit. Raising this value masks "musical noise" (robotic tinkling artifacts) by leaving a smooth, imperceptible layer of background noise.      |
| **VAD Threshold**           | `2.0` – `4.0`     | The energy multiplier required to trigger speech detection. Higher values prevent loud transient noises (like a passing car) from accidentally freezing the noise tracker. |

---

## Engineering Deep-Dive

Building a real-time DSP engine requires navigating strict memory constraints, multi-rate processing architectures, and compiler-specific behaviors.

To read the complete engineering breakdown of how this plugin was built, mathematically verified, and tuned, visit the **[Project Documentation Site](https://delali-adanuty.github.io/NoiseReductionJUCE/)**.

---

**Author:** Van-Dyck Adanuty

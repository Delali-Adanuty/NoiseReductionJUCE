# Real-Time Noise Reducer (Standalone App & Plugin)

A real-time C++ audio processor built with the JUCE framework that utilizes spectral subtraction to attenuate background noise and room hiss from live microphone input.

## Why This Exists

Conventional noise gates abruptly mute audio below a set threshold, leaving continuous background noise audible during active speech. This project estimates the room noise profile from acoustic measurements and generates a dynamic spectral floor to attenuate ambient noise while preserving the harmonic components of the primary voice.

## Overview

This processor operates either as a **Standalone Desktop App** (utilizing your native microphone and audio drivers) or as a traditional VST3/AU plugin within a Digital Audio Workstation (DAW).

Unlike a traditional noise gate, this engine performs continuous time-frequency analysis. It profiles ambient room noise and attenuates it from the incoming frequency spectrum while actively tracking the primary voice.

## Core Features

- **Real-Time FFT Pipeline:** Utilizes high-performance, low-latency frequency domain transformation.
- **Voice Activity Detection (VAD):** Accurately isolates speech to freeze the noise profile dynamically, preventing the algorithm from locking onto digital silence.
- **Asymmetric Noise Tracking:** Adapts to changing environments. The algorithm learns quickly during ambient silence to track background hiss, but updates its profile slowly when speech is detected to prevent vocal cancellation.
- **Overlap-Add (OLA) Reconstruction:** Leverages a windowed analysis/synthesis phase to ensure seamless audio reconstruction.

## Limitations

- **Stationary Noise Assumption:** Optimized for consistent background noise (e.g., HVAC, fans). It cannot effectively attenuate highly transient or unpredictable interference.
- **Musical Noise:** Aggressive oversubtraction settings can introduce robotic, "tinkling" audio artifacts.
- **Initialization:** Requires a brief period of digital silence upon startup to accurately measure the baseline noise profile.
- **Latency:** Processing introduces an inherent delay tied to the FFT block size and Overlap-Add hop size.

---

## Engineering Deep-Dive & Verification

If you are interested in the DSP math, how the plugin was mathematically verified, performance benchmarks (spectrograms and SNR reduction charts), or the challenges of translating MATLAB prototypes into real-time C++ architecture, read the **[Behind the Scenes Documentation](link_to_docs.md)**.

---

**Author:** Van-Dyck Adanuty

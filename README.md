# Real-Time Noise Reducer (Standalone App & Plugin)

A real-time C++ audio processor built with the JUCE framework that uses spectral subtraction to intelligently remove background noise and room hiss from live microphone input.

## Overview

This processor can run completely on its own as a **Standalone Desktop App** (using your computer's native microphone and headphones) or as a traditional VST3/AU plugin inside a Digital Audio Workstation (DAW).

Instead of using a simple noise gate that abruptly cuts off audio, this processor constantly analyzes the frequency spectrum to build a profile of the room's background noise, subtracting it from the incoming signal while leaving the primary voice intact.

## Features

- **Real-Time FFT Processing:** Utilizes `juce::dsp::FFT` for high-speed frequency domain transformation.
- **Asymmetric Noise Tracking:** Intelligently adapts to room noise. It learns quickly when the room is quiet (to track background hiss) but learns extremely slowly when a signal is loud (to prevent muting the speaker's voice).
- **Safe Output Reconstruction:** Bypasses complex Overlap-Add (OLA) phase cancellation by directly reading the valid Hann window reconstruction region from the center of the Inverse FFT.
- **Zero-Latency Safety:** Implements strict memory isolation (separate input, output, and FFT buffers) to prevent digital feedback loops and in-place memory corruption.

## Usage & Tuning Parameters

The noise reduction algorithm is controlled by two primary parameters:

-**Alpha (Over-subtraction Factor):** Default `2.0`.
Controls how aggressively the estimated noise profile is subtracted from the audio.

- _Tip:_ Raise this to remove more noise. If the voice starts sounding thin, hollow, or "chirpy", lower the alpha value. Values above `4.0` generally result in over-processing.
- **Beta (Spectral Floor):** Default `0.01`. Acts as a safety net to prevent frequency bins from dropping entirely to zero.
  - _Tip:_ Raise this if the output starts sounding overly "underwater" or robotic.

## Building from Source

This project uses the JUCE Projucer.

1. Clone this repository.
2. Open `NewProject.jucer` in the Projucer.
3. Ensure your global JUCE paths are set correctly in the Projucer settings.
4. Click "Save and Open in IDE" (Visual Studio for Windows, Xcode for macOS).
5. Build the `Standalone Plugin` or `VST3` target.

_Note: This repository strictly tracks source code. You must build the binaries locally._

## Engineering Deep-Dive

Building a real-time DSP engine requires navigating strict memory constraints, compiler-specific behaviors, and complex audio plumbing.

Read the full engineering breakdown of how this plugin was built, debugged, and optimized on the **[Project Documentation Site](#)**

---

**Author:** Van-Dyck Adanuty
